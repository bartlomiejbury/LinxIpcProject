#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxQueueFdImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcEndpointThreadImpl.h"
#include "LinxIpcEndpointSimpleImpl.h"

//LCOV_EXCL_START
static LinxIpcEndpointPtr createLinxIpEndpointNoThread(const std::string &endpointName) {
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return std::make_shared<LinxIpcEndpointSimpleImpl>(socket);
}

static LinxIpcEndpointPtr createLinxIpEndpointThread(const std::string &endpointName, int maxSize) {
    LinxQueueFdImpl *efd = new LinxQueueFdImpl();
    LinxQueue *queue = new LinxQueueImpl(efd, maxSize);
    auto endpoint = createLinxIpEndpointNoThread(endpointName);
    return std::make_shared<LinxIpcEndpointThreadImpl>(endpoint, queue);
}

LinxIpcServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize) {
    auto endpoint = createLinxIpEndpointThread(endpointName, maxSize);
    return std::make_shared<LinxIpcServerImpl>(endpoint);
}

LinxIpcServerPtr createLinxIpcSimpleServer(const std::string &endpointName) {
    auto endpoint = createLinxIpEndpointNoThread(endpointName);
    return std::make_shared<LinxIpcServerImpl>(endpoint);
}
//LCOV_EXCL_STOP
