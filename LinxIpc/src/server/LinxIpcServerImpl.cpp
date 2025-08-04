#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcSocket.h"
#include "LinxQueueImpl.h"
#include "LinxTrace.h"
#include "LinxIpcPrivate.h"

/*
* Extended server
*/
void LinxIpcServerImpl::task() {

    while (running) {
        LinxMessageIpcPtr msg{};
        std::string from;
        static constexpr int timeoutMs = 100;
        int ret = socket->receive(&msg, &from, timeoutMs);

        if (ret >= 0) {
            msg->setClient(createClient(from));
            if (msg->getReqId() == IPC_PING_REQ) {
                LinxMessageIpc rsp = LinxMessageIpc(IPC_PING_RSP);
                socket->send(rsp, from);
                continue;
            }

            if (queue->add(msg) != 0) {
                LINX_ERROR("Received request on IPC: %s: %d from: %s discarded - queue full", this->serviceName.c_str(),
                          msg->getReqId(), from.c_str());
            }
        }
    }
}

LinxIpcServerImpl::LinxIpcServerImpl(LinxIpcSocket *socket, LinxQueue *queue) {
    assert(queue);
    assert(socket);
    this->serviceName = socket->getName();
    this->socket = socket;
    this->queue = queue;
}

LinxIpcServerImpl::~LinxIpcServerImpl() {
    stop();
    queue->clear();
    delete queue;
    delete socket;
}

void LinxIpcServerImpl::start() {
    
    if (workerThread.joinable()) {
        return;
    }

    running = true;
    workerThread = std::thread([this]() { this->task(); });
}

void LinxIpcServerImpl::stop() {
    if (workerThread.joinable()) {
        running = false;
        workerThread.join();
    }
}

int LinxIpcServerImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    if (!to) {
        LINX_ERROR("Cannot send message to null client");
        return -1;
    }
    return socket->send(message, to->getName());
}

LinxIpcClientPtr LinxIpcServerImpl::createClient(const std::string &serviceName) {
      return std::make_shared<LinxIpcClientImpl>(shared_from_this(), serviceName);
}

LinxMessageIpcPtr LinxIpcServerImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                               const LinxIpcClientPtr &from) {

    std::optional<LinxIpcClientPtr> fromOpt = from != nullptr ? std::make_optional(from) : std::nullopt;
    auto msg = queue->get(timeoutMs, sigsel, fromOpt);
    if (msg != nullptr) {
        LINX_DEBUG("Received request on IPC: %s: %d from: %s", serviceName.c_str(), msg->getReqId(),
                msg->getClient()->getName().c_str());
        return msg;
    }

    return nullptr;
}

int LinxIpcServerImpl::getPollFd() const {
    return queue->getFd();
}

LinxIpcServerPtr createIpcServer(const std::string &serviceName, int maxSize) {
    LinxIpcSocket* socket = createLinxSocket(serviceName);
    if (!socket) {
        return nullptr;
    }

    LinxQueue *queue = createIpcQueue(maxSize);
    return std::make_shared<LinxIpcServerImpl>(socket, queue);
}