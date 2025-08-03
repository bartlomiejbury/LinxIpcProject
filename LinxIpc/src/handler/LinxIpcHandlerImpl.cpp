#include "LinxIpcHandlerImpl.h"
#include "LinxTrace.h"

LinxIpcHandlerImpl::LinxIpcHandlerImpl(const LinxIpcServerPtr &server, const std::map<uint32_t, IpcContainer> &handlers): 
    server(server), 
    handlers(handlers) {
}

LinxIpcHandlerImpl::~LinxIpcHandlerImpl() {
    stop();
}

int LinxIpcHandlerImpl::handleMessage(int timeoutMs) {
    LinxMessageIpcPtr message = receive(timeoutMs, LINX_ANY_SIG, LINX_ANY_FROM);
    if (message) {
        auto it = handlers.find(message->getReqId());
        if (it != handlers.end()) {
            IpcContainer &container = it->second;
            return container.callback(message.get(), container.data);
        } else {            
            LINX_ERROR("No handler for request ID: %d", message->getReqId());
            return 0;
        }
    }
    return -1;// Indicate no message handled
}
    
LinxIpcClientPtr LinxIpcHandlerImpl::createClient(const std::string &serviceName) {
    return server->createClient(serviceName);
}

void LinxIpcHandlerImpl::start() {
    server->start();
}

void LinxIpcHandlerImpl::stop() {
    server->stop();
}

int LinxIpcHandlerImpl::getPollFd() const {
    return server->getPollFd();
}

int LinxIpcHandlerImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    return server->send(message, to);
}
    
LinxMessageIpcPtr LinxIpcHandlerImpl::receive(int timeoutMs, 
                                      const std::vector<uint32_t> &sigsel,
                                      const LinxIpcClientPtr &from) {
    return server->receive(timeoutMs, sigsel, from);
}

LinxIpcHandlerBuilder& LinxIpcHandlerBuilder::registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) {
    handlers.insert({reqId, {callback, data}});
    return *this;
}

LinxIpcHandlerPtr LinxIpcHandlerBuilder::build() {
    LinxIpcServerPtr server = createIpcServer(serverName);
    if (!server) {
        return nullptr;
    }

    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);
    handler->start();
    return handler;
}
