#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcEndpointThreadImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocket.h"
#include "LinxQueue.h"
#include "trace.h"

void LinxIpcEndpointThreadImpl::task() {

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

void *LinxIpcEndpointThreadImpl::threadFunc(void *arg) {
    LinxIpcEndpointThreadImpl *task = (LinxIpcEndpointThreadImpl *)arg;
    task->task();
    return NULL;
}

LinxIpcEndpointThreadImpl::LinxIpcEndpointThreadImpl(const LinxIpcEndpointPtr &endpoint, LinxQueue *queue) {
    assert(queue);
    assert(endpoint);

    this->queue = queue;
    this->endpoint = endpoint;
}

LinxIpcEndpointThreadImpl::~LinxIpcEndpointThreadImpl() {

    stop();
    queue->clear();
    delete queue;
}

void LinxIpcEndpointThreadImpl::start() {

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
    pthread_create(&threadId, &attr, LinxIpcEndpointThreadImpl::threadFunc, this);
    pthread_attr_destroy(&attr);
}

void LinxIpcEndpointThreadImpl::stop() {
    if (running) {
        running = false;
        pthread_cancel(threadId);
        pthread_join(threadId, NULL);
    }
}

LinxMessageIpcPtr LinxIpcEndpointThreadImpl::receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                                               const LinxIpcClientPtr &from) {

    LinxMessageIpcPtr msg = queue->get(timeoutMs, sigsel, from);
    if (msg != nullptr) {
        LOG_DEBUG("Received request on IPC: %s: %d from: %s", endpoint->getName().c_str(), msg->getReqId(),
                msg->getClinet()->getName().c_str());
    }

    return msg;
}

int LinxIpcEndpointThreadImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    return endpoint->send(message, to);
}

LinxIpcClientPtr LinxIpcEndpointThreadImpl::createClient(const std::string &serviceName) {
    return endpoint->createClient(serviceName);
}

int LinxIpcEndpointThreadImpl::getPollFd() const {
    return queue->getFd();
}
