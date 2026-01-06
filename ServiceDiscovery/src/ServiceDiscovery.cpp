#include "ServiceDiscovery.h"
#include "UdpLinx.h"
#include "Messages.h"
#include "trace.h"

ServiceDiscovery::ServiceDiscovery(std::shared_ptr<UdpClient> client, int timeoutMs)
    : client(std::move(client)), defaultTimeoutMs(timeoutMs) {
    auto ret = this->client->connect(INFINITE_TIMEOUT);
    if (!ret) {
        TRACE_ERROR("ServiceDiscovery: Failed to connect UDP client");
    }
}

std::optional<PortInfo> ServiceDiscovery::discover(const std::string &serviceName) {
    auto msg = DiscoveryRequestMessage(serviceName);
    auto rsp = client->sendReceive(msg, defaultTimeoutMs, {DISCOVERY_RSP});
    if (!rsp) {
        TRACE_ERROR("ServiceDiscovery: Discovery request timed out for service '%s'", serviceName.c_str());
        return std::nullopt;
    }
    auto discoveryResponse = DiscoveryResponseMessage::fromRawMessage(*rsp);
    if (discoveryResponse->portInfo.port == 0) {
        TRACE_ERROR("ServiceDiscovery: Service '%s' not found", serviceName.c_str());
        return std::nullopt;
    }

    TRACE_DEBUG("ServiceDiscovery: Discovered service '%s' at %s", serviceName.c_str(), discoveryResponse->portInfo.format().c_str());
    return discoveryResponse->portInfo;
}

bool ServiceDiscovery::registerService(const std::string &serviceName, uint16_t port) {
    auto msg = RegisterRequestMessage(serviceName, port);
    auto rsp = client->sendReceive(msg, defaultTimeoutMs, {REGISTER_RSP});
    if (!rsp) {
        TRACE_ERROR("ServiceDiscovery: Register request timed out for service '%s'", serviceName.c_str());
        return false;
    }
    auto registerResponse = RegisterResponseMessage::fromRawMessage(*rsp);
    if (!registerResponse->success) {
        TRACE_ERROR("ServiceDiscovery: Failed to register service '%s'", serviceName.c_str());
        return false;
    }

    TRACE_DEBUG("ServiceDiscovery: Registered service '%s' on port %u", serviceName.c_str(), port);
    return registerResponse->success;
}

bool ServiceDiscovery::unregisterService(const std::string &serviceName) {
    auto msg = UnregisterRequestMessage(serviceName);
    auto rsp = client->sendReceive(msg, defaultTimeoutMs, {UNREGISTER_RSP});
    if (!rsp) {
        TRACE_ERROR("ServiceDiscovery: Unregister request timed out for service '%s'", serviceName.c_str());
        return false;
    }
    auto unregisterResponse = UnregisterResponseMessage::fromRawMessage(*rsp);
    if (!unregisterResponse->success) {
        TRACE_ERROR("ServiceDiscovery: Failed to unregister service '%s'", serviceName.c_str());
        return false;
    }

    TRACE_DEBUG("ServiceDiscovery: Unregistered service '%s'", serviceName.c_str());
    return unregisterResponse->success;
}

std::shared_ptr<ServiceDiscovery> ServiceDiscovery::create() {
    auto udpClient = UdpFactory::createClient(SERVICE_DISCOVERY_IP, ServiceDiscovery::SERVICE_DISCOVERY_PORT);
    if (!udpClient) {
        TRACE_ERROR("ServiceDiscovery: Failed to create UDP client");
        return nullptr;
    }
    return std::make_shared<ServiceDiscovery>(udpClient);
}