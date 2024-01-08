#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocket.h"
#include "LinxQueue.h"
#include "trace.h"

void LinxIpcServerImpl::task() {

    while (running) {

        auto msg = endpoint->receive(INFINITE_TIMEOUT, LINX_ANY_SIG, LINX_ANY_FROM);
        if (msg) {

            if (msg->getReqId() == IPC_HUNT_REQ) {
                LinxMessageIpc rsp = LinxMessageIpc(IPC_HUNT_RSP);
                msg->getClient()->send(rsp);
                continue;
            }

            auto from = msg->getClient()->getName();
            if (queue->add(msg) != 0) {
                LOG_ERROR("Received request on IPC: %s: %d from: %s discarded - queue full", socket->getName().c_str(),
                          msg->getReqId(), from.c_str());
            }
        }
    }
}

void *LinxIpcServerImpl::threadFunc(void *arg) {
    LinxIpcServerImpl *task = (LinxIpcServerImpl *)arg;
    task->task();
    return NULL;
}

LinxIpcServerImpl::LinxIpcServerImpl(const LinxIpcEndpointPtr &endpoint, LinxQueue *queue) {
    assert(queue);
    assert(endpoint);

    this->queue = queue;
    this->endpoint = endpoint;
}

LinxIpcServerImpl::~LinxIpcServerImpl() {
    stop();
    queue->clear();
    delete queue;
}

void LinxIpcServerImpl::start() {

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
    pthread_create(&threadId, &attr, LinxIpcServerImpl::threadFunc, this);
    pthread_attr_destroy(&attr);
}

void LinxIpcServerImpl::stop() {
    if (running) {
        running = false;
        pthread_cancel(threadId);
        pthread_join(threadId, NULL);
    }
}

LinxMessageIpcPtr LinxIpcServerImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                               const LinxIpcClientPtr &from) {

    LinxMessageIpcPtr msg = queue->get(timeoutMs, sigsel, from);
    if (msg != nullptr) {
        LOG_DEBUG("Received request on IPC: %s: %d from: %s", endpoint->getName().c_str(), msg->getReqId(),
                msg->getClinet()->getName().c_str());
    }

    return msg;
}

int LinxIpcServerImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    return endpoint->send(message, to);
}

LinxIpcClientPtr LinxIpcServerImpl::createClient(const std::string &serviceName) {
    return endpoint->createClient(serviceName);
}

int LinxIpcServerImpl::getPollFd() const {
    return queue->getFd();
}

void LinxIpcServerImpl::registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) {
    this->handlers.insert({reqId, {callback, data}});
};

int LinxIpcServerImpl::handleMessage(int timeoutMs) {

    LinxMessageIpcPtr msg = endpoint->receive(timeoutMs, LINX_ANY_SIG, LINX_ANY_FROM);
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
