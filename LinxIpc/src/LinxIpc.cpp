#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxQueueFdImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"

//LCOV_EXCL_START
LinxIpcServerPtr createLinxIpcSimpleServer(const std::string &endpointName) {
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcSimpleServerImpl>(socket);
}

LinxIpcExtendedServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    LinxQueueFdImpl *efd = new LinxQueueFdImpl();
    LinxQueue *queue = new LinxQueueImpl(efd, maxSize);
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcExtendedServerImpl>(socket, queue);
}
//LCOV_EXCL_STOP
