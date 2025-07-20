#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxIpcEventFdImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"
#include <exception>
#include <stdexcept>

//LCOV_EXCL_START
LinxIpcServerPtr createLinxIpcSimpleServer(const std::string &endpointName) {
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcSimpleServerImpl>(socket);
}

LinxIpcServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    LinxIpcEventFdImpl *efd = new LinxIpcEventFdImpl();
    LinxQueue *queue = new LinxQueueImpl(efd, maxSize);
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcExtendedServerImpl>(socket, queue);
}

LinxIpcHandlerBuilder::LinxIpcHandlerBuilder(const std::string &serverName): serverName(serverName) {
}

LinxIpcHandlerBuilder &LinxIpcHandlerBuilder::setSimpleServer() {
    this->server = createLinxIpcSimpleServer(serverName);
    return *this;
}

LinxIpcHandlerBuilder &LinxIpcHandlerBuilder::setExtendedServer(int maxSize) {
    this->server = createLinxIpcServer(serverName, maxSize);
    return *this;
}

LinxIpcHandlerPtr LinxIpcHandlerBuilder::build() {
    if (!server) {
        throw std::runtime_error("Server not set. Use setSimpleServer() or setExtendedServer() before building.");
    }

    server->start();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server);
    return handler;
}


//LCOV_EXCL_STOP
