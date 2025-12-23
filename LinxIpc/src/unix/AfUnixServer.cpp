#include <cassert>
#include "AfUnixClient.h"
#include "AfUnixEndpoint.h"
#include "AfUnixServer.h"
#include "AfUnixSocket.h"
#include "LinxEventFd.h"
#include "LinxIpc.h"
#include "LinxMessageIds.h"
#include "LinxQueue.h"
#include "LinxTrace.h"

void AfUnixServer::task() {

    while (running) {
        LinxMessagePtr msg{};
        std::string from;

        static constexpr int timeoutMs = 100;
        int ret = socket->receive(&msg, &from, timeoutMs);

        if (ret >= 0) {
            auto reqId = msg->getReqId();
            if (reqId == IPC_PING_REQ) {
                LinxMessage rsp = LinxMessage(IPC_PING_RSP);
                socket->send(rsp, from);
                continue;
            }

            auto container =  std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
                .message = std::move(msg),
                .context = std::make_unique<AfUnixEndpoint>(this->socket, shared_from_this(), from)
            });
            if (queue->add(std::move(container)) != 0) {
                LINX_ERROR("Received request on IPC: %s: 0x%x from: %s discarded - queue full", this->serviceName.c_str(),
                          reqId, from.c_str());
            }
        }
    }
}

AfUnixServer::AfUnixServer(const std::shared_ptr<AfUnixSocket> &socket, std::unique_ptr<LinxQueue> &&queue, const std::string &serviceName) {
    assert(queue);
    assert(socket);
    this->serviceName = socket->getName();
    this->socket = std::move(socket);
    this->queue = std::move(queue);
}

AfUnixServer::~AfUnixServer() {
    stop();
    queue->clear();
}

bool AfUnixServer::start() {

    if (running) {
        return true;
    }

    auto ret = this->socket->open();
    if (!ret) {
        return false;
    }

    running = true;
    workerThread = std::thread([this]() { this->task(); });
    return true;
}

void AfUnixServer::stop() {
    if (workerThread.joinable()) {
        running = false;
        workerThread.join();
    }
}

LinxReceivedMessageSharedPtr AfUnixServer::receive(int timeoutMs,
                                                const std::vector<uint32_t> &sigsel,
                                                const LinxReceiveContextSharedPtr &from ) {

    LinxReceiveContextOpt fromOpt = from != nullptr ? std::make_optional(from) : std::nullopt;
    auto recvMsg = queue->get(timeoutMs, sigsel, fromOpt);
    if (recvMsg != nullptr) {
        auto reqId = recvMsg->message->getReqId();
        LINX_DEBUG("Received request on IPC: %s: 0x%x", serviceName.c_str(), reqId);
        return recvMsg;
    }

    return nullptr;
}

int AfUnixServer::getPollFd() const {
    return queue->getFd();
}

LinxReceiveContextSharedPtr AfUnixServer::createContext(const std::string &from) const {
    auto nonConstThis = std::const_pointer_cast<AfUnixServer>(shared_from_this());
    return std::make_shared<AfUnixEndpoint>(this->socket, nonConstThis, from);
}

std::shared_ptr<AfUnixServer> AfUnixServer::create(const std::string &serviceName, size_t queueSize) {
    auto socket = std::make_shared<AfUnixSocket>(serviceName);
    auto efd = std::make_unique<LinxEventFd>();
    auto queue = std::make_unique<LinxQueue>(std::move(efd), queueSize);
    return std::make_shared<AfUnixServer>(socket, std::move(queue), serviceName);
}