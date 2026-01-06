#pragma once

#include "UdpLinx.h"
#include <unordered_map>

class ServiceRepository {
  public:
    ServiceRepository() = default;
    ~ServiceRepository() = default;

    bool addService(const std::string &serviceName, const std::string &ip, uint16_t port);
    bool removeService(const std::string &serviceName);
    std::optional<PortInfo> getService(const std::string &serviceName);

private:
    std::unordered_map<std::string, PortInfo> services;
};