#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxIpcEventFdImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"


//LCOV_EXCL_START
LinxIpcServerPtr createLinxIpcSimpleServer(const std::string &endpointName) {
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcSimpleServerImpl>(socket);
}

LinxIpcExtendedServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    LinxIpcEventFdImpl *efd = new LinxIpcEventFdImpl();
    LinxQueue *queue = new LinxQueueImpl(efd, maxSize);
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcExtendedServerImpl>(socket, queue);
}

LinxIpcHandlerPtr createLinxIpcHandler(const LinxIpcServerPtr &server) {
    return std::make_shared<LinxIpcHandlerImpl>(server);
}

//LCOV_EXCL_STOP
