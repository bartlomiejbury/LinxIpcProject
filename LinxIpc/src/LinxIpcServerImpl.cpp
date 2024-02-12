#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocket.h"
#include "LinxQueue.h"
#include "trace.h"

LinxIpcSimpleServerImpl::LinxIpcSimpleServerImpl(LinxIpcSocket *socket) {
    assert(socket);
    this->socket = socket;

    registerCallback(IPC_HUNT_REQ, [](LinxMessageIpc *msg, void *data){
        LinxMessageIpc rsp = LinxMessageIpc(IPC_HUNT_RSP);
        return msg->getClient()->send(rsp);
    }, nullptr);
}

LinxIpcSimpleServerImpl::~LinxIpcSimpleServerImpl() {
    delete socket;
}

int LinxIpcSimpleServerImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    if (to == nullptr) {
        return -1;
    }
        printf("!!!!!%p\n", socket);
    return socket->send(message, to->getName());
}

int LinxIpcSimpleServerImpl::getPollFd() const {
    return socket->getFd();
}

LinxIpcClientPtr LinxIpcSimpleServerImpl::createClient(const std::string &serviceName) {
    return std::make_shared<LinxIpcClientImpl>(shared_from_this(), serviceName);
}

int LinxIpcSimpleServerImpl::handleMessage(int timeoutMs) {
    LinxMessageIpcPtr msg = receive(timeoutMs, LINX_ANY_SIG, LINX_ANY_FROM);
    if (msg == nullptr) {
        return -1;
    }

    int ret = 0;
    auto it = this->handlers.find(msg->getReqId());
    if (it != this->handlers.end()) {
        auto container = it->second;
        ret = container.callback(msg.get(), container.data);
    } else {
        LOG_INFO("Received unknown request on IPC: %s: %d from: %s", socket->getName().c_str(), msg->getReqId(),
                 msg->getClient()->getName().c_str());
    }

    return ret;
}

void LinxIpcSimpleServerImpl::registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) {
        this->handlers.insert({reqId, {callback, data}});
}

LinxMessageIpcPtr LinxIpcSimpleServerImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel, const LinxIpcClientPtr &client) {

    LinxMessageIpcPtr msg{};
    std::string from;
    int ret = socket->receive(&msg, &from, timeoutMs);
    if (ret >= 0) {

        msg->setClient(createClient(from));
        auto predicate = [&sigsel, &client](LinxMessageIpcPtr &msg) {
        if (!client || msg->getClient()->getName() == client->getName()) {
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

            if (msg->getReqId() == IPC_HUNT_REQ) {
                LinxMessageIpc rsp = LinxMessageIpc(IPC_HUNT_RSP);
                socket->send(rsp, from);
                continue;
            }

            if (queue->add(msg, from) != 0) {
                LOG_ERROR("Received request on IPC: %s: %d from: %s discarded - queue full", socket->getName().c_str(),
                          msg->getReqId(), from.c_str());
            }
        }
    }
}

void *LinxIpcExtendedServerImpl::threadFunc(void *arg) {
    LinxIpcExtendedServerImpl *task = (LinxIpcExtendedServerImpl *)arg;
    task->task();
    return NULL;
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

    if (running) {
        return;
    }

    running = true;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);

    struct sched_param param;
    param.sched_priority = 0;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&threadId, &attr, LinxIpcExtendedServerImpl::threadFunc, this);
    pthread_attr_destroy(&attr);
}

void LinxIpcExtendedServerImpl::stop() {
    if (running) {
        running = false;
        pthread_cancel(threadId);
        pthread_join(threadId, NULL);
    }
}

LinxMessageIpcPtr LinxIpcExtendedServerImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                               const LinxIpcClientPtr &from) {

    std::optional<std::string> fromOpt = from != nullptr ? std::make_optional(from->getName()) : std::nullopt;
    LinxQueueElement container = queue->get(timeoutMs, sigsel, fromOpt);
    if (container != nullptr) {
        auto msg = std::get<LinxMessageIpcPtr>(*container);
        auto client = std::get<std::string>(*container);

        msg->setClient(createClient(client));
        LOG_DEBUG("Received request on IPC: %s: %d from: %s", socket->getName().c_str(), msg->getReqId(),
                msg->getClient()->getName().c_str());
        return msg;
    }

    return nullptr;
}

int LinxIpcExtendedServerImpl::getPollFd() const {
    return queue->getFd();
}
