#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcEndpointImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocket.h"
#include "LinxQueue.h"
#include "trace.h"

void LinxIpcEndpointImpl::task() {

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

void *LinxIpcEndpointImpl::threadFunc(void *arg) {
    LinxIpcEndpointImpl *task = (LinxIpcEndpointImpl *)arg;
    task->task();
    return NULL;
}

LinxIpcEndpointImpl::LinxIpcEndpointImpl(LinxQueue *queue, LinxIpcSocket *socket) {
    assert(queue);
    assert(socket);

    this->queue = queue;
    this->socket = socket;
}

LinxIpcEndpointImpl::~LinxIpcEndpointImpl() {

    stop();
    queue->clear();
    delete queue;
    delete socket;
}

void LinxIpcEndpointImpl::start() {

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
    pthread_create(&threadId, &attr, LinxIpcEndpointImpl::threadFunc, this);
    pthread_attr_destroy(&attr);
}

void LinxIpcEndpointImpl::stop() {
    if (running) {
        running = false;
        pthread_cancel(threadId);
        pthread_join(threadId, NULL);
    }
}

LinxMessageIpcPtr LinxIpcEndpointImpl::receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                                               const LinxIpcClientPtr &from) {

    std::optional<std::string> fromOpt = from != nullptr ? std::make_optional(from->getName()) : std::nullopt;
    std::shared_ptr<LinxQueueContainer> container = queue->get(timeoutMs, sigsel, fromOpt);
    if (container == nullptr) {
        return nullptr;
    }

    LinxMessageIpcPtr msg = container->message;
    msg->setClient(createClient(container->from));

    LOG_DEBUG("Received request on IPC: %s: %d from: %s", socket->getName().c_str(), msg->getReqId(),
              container->from.c_str());

    return msg;
}

int LinxIpcEndpointImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    if (to == nullptr) {
        return -1;
    }
    return this->socket->send(message, to->getName());
}

LinxIpcClientPtr LinxIpcEndpointImpl::createClient(const std::string &serviceName) {
    return std::make_shared<LinxIpcClientImpl>(shared_from_this(), serviceName);
}

int LinxIpcEndpointImpl::getPollFd() const {
    return queue->getFd();
}
