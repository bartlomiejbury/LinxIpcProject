#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxIpcEndpointImpl.h"

LinxIpcEndpointPtr createLinxIpcEndpoint(const std::string &endpointName, int maxSize) {
    LinxQueue *queue = new LinxQueueImpl(maxSize);
    LinxIpcSocket *socket = new LinxIpcSocketImpl(endpointName);
    return LinxIpcEndpointPtr(new LinxIpcEndpointImpl(queue, socket));
}
