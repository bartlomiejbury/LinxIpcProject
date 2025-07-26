#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocket.h"
#include "LinxQueue.h"
#include "LinxTrace.h"
#include "LinxIpcPrivate.h"

LinxIpcSimpleServerImpl::LinxIpcSimpleServerImpl(LinxIpcSocket *socket) {
    assert(socket);
    this->socket = socket;
}


LinxIpcSimpleServerImpl::~LinxIpcSimpleServerImpl() {
    delete socket;
}

void LinxIpcSimpleServerImpl::start() {};
void LinxIpcSimpleServerImpl::stop() {};

int LinxIpcSimpleServerImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    if (to == nullptr) {
        return -1;
    }
    return socket->send(message, to->getName());
}

int LinxIpcSimpleServerImpl::getPollFd() const {
    return socket->getFd();
}

LinxIpcClientPtr LinxIpcSimpleServerImpl::createClient(const std::string &serviceName) {
    return std::make_shared<LinxIpcClientImpl>(shared_from_this(), serviceName);
}

LinxMessageIpcPtr LinxIpcSimpleServerImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel, const LinxIpcClientPtr &client) {

    LinxMessageIpcPtr msg{};
    std::string from;
    int ret = socket->receive(&msg, &from, timeoutMs);
    if (ret >= 0) {

        msg->setClient(createClient(from));
        auto predicate = [&sigsel, &client](LinxMessageIpcPtr &msg) {
            if (!client || *msg->getClient() == *client) {
                uint32_t reqId = msg->getReqId();
                return sigsel.size() == 0 || std::find_if(sigsel.begin(), sigsel.end(),
                                                        [reqId](uint32_t id) { return id == reqId; }) != sigsel.end();
            }

            return false;
        };

        return predicate(msg) ? msg : nullptr;
    }

    return nullptr;
}

/*
* Extended server
*/
void LinxIpcExtendedServerImpl::task() {

    while (running) {

        LinxMessageIpcPtr msg{};
        std::string from;

        int ret = socket->receive(&msg, &from, INFINITE_TIMEOUT);
        if (ret >= 0) {

            if (msg->getReqId() == IPC_PING_REQ) {
                LinxMessageIpc rsp = LinxMessageIpc(IPC_PING_RSP);
                socket->send(rsp, from);
                continue;
            }

            msg->setClient(createClient(from));
            if (queue->add(msg) != 0) {
                LINX_ERROR("Received request on IPC: %s: %d from: %s discarded - queue full", socket->getName().c_str(),
                          msg->getReqId(), from.c_str());
            }
        }
    }
}

LinxIpcExtendedServerImpl::LinxIpcExtendedServerImpl(LinxIpcSocket *socket, LinxQueue *queue): LinxIpcSimpleServerImpl(socket) {
    assert(queue);
    this->queue = queue;
}

LinxIpcExtendedServerImpl::~LinxIpcExtendedServerImpl() {
    stop();
    queue->clear();
    delete queue;
}

void LinxIpcExtendedServerImpl::start() {

    if (workerThread.joinable()) {
        return;
    }

    running = true;
    workerThread = std::thread([this]() { this->task(); });
}

void LinxIpcExtendedServerImpl::stop() {
    if (workerThread.joinable()) {
        running = false;
        workerThread.join();
    }
}

LinxMessageIpcPtr LinxIpcExtendedServerImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                               const LinxIpcClientPtr &from) {

    std::optional<LinxIpcClientPtr> fromOpt = from != nullptr ? std::make_optional(from) : std::nullopt;
    auto msg = queue->get(timeoutMs, sigsel, fromOpt);
    if (msg != nullptr) {
        LINX_DEBUG("Received request on IPC: %s: %d from: %s", socket->getName().c_str(), msg->getReqId(),
                msg->getClient()->getName().c_str());
        return msg;
    }

    return nullptr;
}

int LinxIpcExtendedServerImpl::getPollFd() const {
    return queue->getFd();
}

/*
* IPC handler
*/
LinxIpcHandlerImpl::LinxIpcHandlerImpl(LinxIpcServerPtr server, std::map<uint32_t, IpcContainer> &handlers) {
    assert(server);
    this->server = server;
    this->handlers = handlers;
}

int LinxIpcHandlerImpl::handleMessage(int timeoutMs) {
    LinxMessageIpcPtr msg = server->receive(timeoutMs, LINX_ANY_SIG, LINX_ANY_FROM);
    if (msg == nullptr) {
        return -1;
    }

    int ret = 0;
    auto it = this->handlers.find(msg->getReqId());
    if (it != this->handlers.end()) {
        auto container = it->second;
        ret = container.callback(msg.get(), container.data);
    } else {
        LINX_INFO("Received unknown IPC request: %d from: %s", msg->getReqId(),
                 msg->getClient()->getName().c_str());
    }

    return ret;
}

LinxIpcClientPtr LinxIpcHandlerImpl::createClient(const std::string &serviceName) {
    return server->createClient(serviceName);
}