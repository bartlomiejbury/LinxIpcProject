#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxIpcEventFdImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"
#include <exception>
#include <stdexcept>

//LCOV_EXCL_START
static LinxIpcServerPtr createLinxIpcSimpleServer(const std::string &endpointName) {
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcSimpleServerImpl>(socket);
}

static LinxIpcServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    LinxIpcEventFdImpl *efd = new LinxIpcEventFdImpl();
    LinxQueue *queue = new LinxQueueImpl(efd, maxSize);
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcExtendedServerImpl>(socket, queue);
}

LinxIpcHandlerBuilder LinxIpcHandlerBuilder::Simple(const std::string &serverName) {
    LinxIpcHandlerBuilder builder = LinxIpcHandlerBuilder();
    builder.type = HandlerType::Simple;
    builder.serverName = serverName;
    return builder;
}
LinxIpcHandlerBuilder LinxIpcHandlerBuilder::Extended(const std::string &serverName, int maxSize) {
    LinxIpcHandlerBuilder builder = LinxIpcHandlerBuilder();
    builder.type = HandlerType::Extended;
    builder.maxSize = maxSize;
    builder.serverName = serverName;
    return builder;
}

LinxIpcHandlerBuilder& LinxIpcHandlerBuilder::registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) {
    handlers.insert({reqId, {callback, data}});
    return *this;
}

LinxIpcHandlerPtr LinxIpcHandlerBuilder::build() {
    LinxIpcServerPtr server;
    if (type == HandlerType::Simple) {
        server = createLinxIpcSimpleServer(serverName);
    } else if (type == HandlerType::Extended) {
        server = createLinxIpcServer(serverName, maxSize);
    }

    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);
    server->start();
    return handler;
}


//LCOV_EXCL_STOP
