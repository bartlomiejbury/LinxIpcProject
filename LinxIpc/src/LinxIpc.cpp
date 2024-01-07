#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxQueueFdImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcEndpointImpl.h"

//LCOV_EXCL_START
static LinxIpcEndpointPtr createLinxIpEndpointThread(const std::string &endpointName, int maxSize) {
    LinxQueueFdImpl *efd = new LinxQueueFdImpl();
    LinxQueue *queue = new LinxQueueImpl(efd, maxSize);
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcEndpointImpl>(queue, socket);
}

LinxIpcServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    auto endpoint = createLinxIpEndpointThread(endpointName, maxSize);
    return std::make_shared<LinxIpcServerImpl>(endpoint);
}
//LCOV_EXCL_STOP
