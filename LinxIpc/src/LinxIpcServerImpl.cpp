#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "trace.h"

LinxIpcServerImpl::LinxIpcServerImpl(LinxQueue *queue, LinxIpcSocket *socket) : LinxIpcEndpointImpl(queue, socket) {}

void LinxIpcServerImpl::registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) {
    this->handlers.insert({reqId, {callback, data}});
};

int LinxIpcServerImpl::handleMessage(int timeoutMs) {

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
