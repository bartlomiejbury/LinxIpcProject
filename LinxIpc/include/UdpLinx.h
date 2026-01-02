#pragma once

#include "GenericServer.h"
#include "GenericClient.h"

class UdpSocket;

namespace UdpFactory {
    bool isMulticastIp(const std::string &ip);
    bool isBroadcastIp(const std::string &ip);
}

// UDP-specific identifier type
class PortInfo : public IIdentifier {
  public:
    std::string ip = "0.0.0.0";
    uint16_t port = 0;
    bool isRestrictedIp = false;

    PortInfo() {}
    PortInfo(const std::string &ip, uint16_t port) : ip(ip), port(port) {
        isRestrictedIp = UdpFactory::isBroadcastIp(ip) || UdpFactory::isMulticastIp(ip);
    }

    std::string format() const override {
        return ip + ":" + std::to_string(port);
    }

    bool isEqual(const IIdentifier &other) const override {
        const auto *otherPort = dynamic_cast<const PortInfo*>(&other);
        if (isRestrictedIp || otherPort->isRestrictedIp) {
            return port == otherPort->port;
        }
        return ip == otherPort->ip && port == otherPort->port;
    }
};

// Socket traits specialization
template<>
struct SocketTraits<UdpSocket> {
    using Identifier = PortInfo;
};


using UdpServer = GenericServer<UdpSocket>;
using UdpClient = GenericClient<UdpSocket>;

namespace UdpFactory {
    std::shared_ptr<UdpServer> createMulticastServer(const std::string &multicastIp, uint16_t port, size_t queueSize = LINX_DEFAULT_QUEUE_SIZE);
    std::shared_ptr<UdpServer> createServer(uint16_t port, size_t queueSize = LINX_DEFAULT_QUEUE_SIZE);
    std::shared_ptr<UdpClient> createClient(const std::string &ip, uint16_t port);
}
