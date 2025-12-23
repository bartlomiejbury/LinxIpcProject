#include "LinxIpc.h"
#include "LinxTrace.h"

LinxIpcHandler::LinxIpcHandler(const std::shared_ptr<LinxServer> &server):
    server(server) {
}

LinxIpcHandler::~LinxIpcHandler() {
    stop();
}

LinxIpcHandler& LinxIpcHandler::registerCallback(uint32_t reqId, const LinxIpcCallback &callback, void *data) {
    IpcContainer container{callback, data};
    handlers[reqId] = container;
    return *this;
}
int LinxIpcHandler::handleMessage(int timeoutMs) {
    auto recvMsg = receive(timeoutMs, LINX_ANY_SIG, LINX_ANY_FROM);
    if (recvMsg) {
        auto reqId = recvMsg->message->getReqId();
        auto it = handlers.find(reqId);
        if (it != handlers.end()) {
            IpcContainer &container = it->second;
            return container.callback(recvMsg, container.data);
        } else {
            LINX_ERROR("No handler for request ID: 0x%x", reqId);
            return 0;
        }
    }
    return -1;// Indicate no message handled
}

bool LinxIpcHandler::start() {
    return server->start();
}

void LinxIpcHandler::stop() {
    server->stop();
}

int LinxIpcHandler::getPollFd() const {
    return server->getPollFd();
}

LinxReceivedMessageSharedPtr LinxIpcHandler::receive(int timeoutMs,
                                      const std::vector<uint32_t> &sigsel,
                                       const LinxReceiveContextSharedPtr &from) {
    return server->receive(timeoutMs, sigsel, from);
}
