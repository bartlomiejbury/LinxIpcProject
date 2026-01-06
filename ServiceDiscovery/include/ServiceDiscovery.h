#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include "UdpLinx.h"
#include "Messages.h"

class ServiceDiscovery {
  public:
    ServiceDiscovery(std::shared_ptr<UdpClient> client, int timeoutMs = 10000);
    ~ServiceDiscovery() = default;

    std::optional<PortInfo> discover(const std::string &serviceName);
    bool registerService(const std::string &serviceName, uint16_t port);
    bool unregisterService(const std::string &serviceName);

    static std::shared_ptr<ServiceDiscovery> create();

    static constexpr uint16_t SERVICE_DISCOVERY_PORT = 12345;
    static constexpr const char* SERVICE_DISCOVERY_IP = "239.255.255.250";

  private:
    std::shared_ptr<UdpClient> client;
    int defaultTimeoutMs = 10000;
};