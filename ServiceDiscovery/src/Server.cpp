#include "ServiceRepository.h"
#include "ServiceDiscovery.h"
#include "LinxIpc.h"
#include "trace.h"
#include <csignal>
#include <atomic>

std::atomic<bool> running(true);
std::shared_ptr<LinxIpcHandler> sdServer;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        TRACE_INFO("Received SIGINT, shutting down...");
        running = false;
        if (sdServer) {
            sdServer->stop();
        }
    }
}

int processDiscoveryRequest(const LinxReceivedMessageSharedPtr &msg, void *data) {
    TRACE_DEBUG("ServiceDiscoveryServer: Processing discovery request");
    auto discoveryRequest = DiscoveryRequestMessage::fromRawMessage(*msg->message);
    auto repository = static_cast<ServiceRepository*>(data);
    auto service = repository->getService(discoveryRequest->serviceName);

    std::string serviceIp;
    uint16_t servicePort;

    if (service) {
        serviceIp = service->ip;
        servicePort = service->port;
        TRACE_DEBUG("ServiceDiscoveryServer: Found service '%s' at %s:%u",
            discoveryRequest->serviceName.c_str(),
            serviceIp.c_str(),
            servicePort);
    } else {
        serviceIp = "";
        servicePort = 0;
        TRACE_DEBUG("ServiceDiscoveryServer: Service '%s' not found",
            discoveryRequest->serviceName.c_str());
    }

    DiscoveryResponseMessage responseMsg(serviceIp, servicePort);
    msg->sendResponse(responseMsg);
    return 0;
}

int processRegisterRequest(const LinxReceivedMessageSharedPtr &msg, void *data) {
    TRACE_DEBUG("ServiceDiscoveryServer: Processing register request");
    auto registerRequest = RegisterRequestMessage::fromRawMessage(*msg->message);
    auto portInfo = dynamic_cast<PortInfo*>(msg->from.get());
    auto repository = static_cast<ServiceRepository*>(data);

    uint16_t port = 0;
    std::string ip = portInfo->ip;
    if (registerRequest->fromSource) {
        port = portInfo->port;
    } else {
        port = registerRequest->port;
    }

    TRACE_DEBUG("ServiceDiscoveryServer: Registering service '%s' on ip: %s, port %u",
        registerRequest->serviceName.c_str(),
        ip.c_str(),
        port);

    repository->addService(registerRequest->serviceName, ip, port);
    RegisterResponseMessage responseMsg(true);
    msg->sendResponse(responseMsg);
    return 0;
}

int processUnregisterRequest(const LinxReceivedMessageSharedPtr &msg, void *data) {
    TRACE_DEBUG("ServiceDiscoveryServer: Processing unregister request");
    auto unregisterRequest = UnregisterRequestMessage::fromRawMessage(*msg->message);
    auto repository = static_cast<ServiceRepository*>(data);
    bool success = repository->removeService(unregisterRequest->serviceName);

    UnregisterResponseMessage responseMsg(success);
    msg->sendResponse(responseMsg);
    return 0;
}

int main() {
    // Register signal handler
    std::signal(SIGINT, signalHandler);

    auto server = UdpFactory::createMulticastServer(ServiceDiscovery::SERVICE_DISCOVERY_IP, ServiceDiscovery::SERVICE_DISCOVERY_PORT, 100);
    auto repository = std::make_shared<ServiceRepository>();
    sdServer = std::make_shared<LinxIpcHandler>(server);

    sdServer->registerCallback(DISCOVERY_REQ, processDiscoveryRequest, (void*)repository.get());
    sdServer->registerCallback(REGISTER_REQ, processRegisterRequest, (void*)repository.get());
    sdServer->registerCallback(UNREGISTER_REQ, processUnregisterRequest, (void*)repository.get());

    TRACE_INFO("Starting Service Discovery Server...");
    sdServer->start();
    while (running) {
        sdServer->handleMessage();
    }

    TRACE_INFO("Service Discovery Server stopped");
    return 0;
}