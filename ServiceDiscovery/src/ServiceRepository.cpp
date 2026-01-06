#include "ServiceRepository.h"

bool ServiceRepository::addService(const std::string &serviceName, const std::string &ip, uint16_t port) {
    PortInfo portInfo{ip, port};
    services[serviceName] = portInfo;
    return true;
}

bool ServiceRepository::removeService(const std::string &serviceName) {
    return services.erase(serviceName) > 0;
}

std::optional<PortInfo> ServiceRepository::getService(const std::string &serviceName) {
    auto it = services.find(serviceName);
    if (it != services.end()) {
        return it->second;
    }
    return std::nullopt;
}
