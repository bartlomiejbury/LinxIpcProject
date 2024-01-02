#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"

//LCOV_EXCL_START
LinxIpcServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    LinxQueue *queue = new LinxQueueImpl(maxSize);
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return LinxIpcServerPtr(new LinxIpcServerImpl(queue, socket));
}
//LCOV_EXCL_STOP
