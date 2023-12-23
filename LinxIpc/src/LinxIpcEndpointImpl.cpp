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

        int ret = socket->receive(&msg, &from, -1);
        if (ret >= 0) {

            if (msg->getReqId() == IPC_HUNT_REQ) {
                LinxMessageIpc rsp = LinxMessageIpc(IPC_HUNT_RSP);
                socket->send(&rsp, from);
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
    this->queue = queue;
    this->socket = socket;
    running = true;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    struct sched_param param;
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    param.sched_priority = 0;

    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&threadId, &attr, LinxIpcEndpointImpl::threadFunc, this);
    pthread_attr_destroy(&attr);
}

LinxIpcEndpointImpl::~LinxIpcEndpointImpl() {
    if (running) {
        running = false;
        pthread_cancel(threadId);
        pthread_join(threadId, NULL);
    }

    queue->clear();

    delete queue;
    delete socket;
}

void LinxIpcEndpointImpl::registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) {
    this->handlers.insert({reqId, {callback, data}});
};

LinxMessageIpcPtr LinxIpcEndpointImpl::receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel) {
    return receive(timeoutMs, sigsel, LINX_ANY_FROM);
}

LinxMessageIpcPtr LinxIpcEndpointImpl::receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClientPtr from) {

    std::optional<std::string> fromOpt = from != nullptr ? std::make_optional(from->getName()) : std::nullopt;
    std::shared_ptr<LinxQueueContainer> container = queue->get(timeoutMs, sigsel, fromOpt);
    if (container == nullptr) {
        return nullptr;
    }

    LinxMessageIpcPtr msg = container->message;
    msg->setClient(std::make_shared<LinxIpcClientImpl>(shared_from_this(), container->from));

    LOG_DEBUG("Received request on IPC: %s: %d from: %s", socket->getName().c_str(),
        msg->getReqId(), container->from.c_str());

    return msg;
}

int LinxIpcEndpointImpl::receive() {

    LinxMessageIpcPtr msg = receive(IMMEDIATE_TIMEOUT, LINX_ANY_SIG);
    if (msg == nullptr) {
        return -1;
    }

    int ret = 0;
    auto it = this->handlers.find(msg->getReqId());
    if (it != this->handlers.end()) {
        auto container = it->second;
        ret = container.callback(msg.get(), container.data);
    } else {
        LOG_INFO("Received unknown request on IPC: %s: %d from: %s", socket->getName().c_str(),
            msg->getReqId(), msg->getClient()->getName().c_str());
    }

    return ret;
}

int LinxIpcEndpointImpl::send(const LinxMessageIpc *message, LinxIpcClientPtr to) {
    return this->socket->send(message, to->getName());
}

LinxIpcClientPtr LinxIpcEndpointImpl::createClient(const std::string &serviceName) {
    return std::make_shared<LinxIpcClientImpl>(shared_from_this(), serviceName);
}

int LinxIpcEndpointImpl::getQueueSize() {
    return queue->size();
}

int LinxIpcEndpointImpl::getFd() {
    return socket->getFd();
}
