#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxQueueFdImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcEndpointImpl.h"

//LCOV_EXCL_START
static LinxIpcEndpointPtr createLinxIpEndpoint(const std::string &endpointName) {
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcEndpointImpl>(socket);
}

LinxIpcServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    LinxQueueFdImpl *efd = new LinxQueueFdImpl();
    LinxQueue *queue = new LinxQueueImpl(efd, maxSize);
    auto endpoint = createLinxIpEndpoint(endpointName);
    return std::make_shared<LinxIpcServerImpl>(endpoint, queue);
}
//LCOV_EXCL_STOP
