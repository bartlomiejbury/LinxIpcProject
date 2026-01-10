#include <sstream>
#include <random>
#include "UdpSocket.h"
#include "LinxEventFd.h"
#include "LinxQueue.h"
#include "LinxTrace.h"
#include "GenericServer.tpp"
#include "GenericClient.tpp"

namespace UdpFactory {

std::shared_ptr<UdpServer> createMulticastServer(const std::string &multicastIp, uint16_t port, size_t queueSize) {

    if (!isMulticastIp(multicastIp)) {
        LINX_ERROR("IP address is not multicast: %s", multicastIp.c_str());
        return nullptr;
    }

        auto socket = std::make_shared<UdpSocket>();
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open UDP socket for server on port: %d", port);
        return nullptr;
    }
    if (socket->bind(port, multicastIp) < 0) {
        LINX_ERROR("Failed to bind UDP socket for server on port: %d", port);
        return nullptr;
    }
    if (socket->joinMulticastGroup(multicastIp) < 0) {
        LINX_ERROR("Failed to join multicast group: %s", multicastIp.c_str());
        return nullptr;
    }

    std::string serverId = multicastIp + ":" + std::to_string(port);
    auto efd = std::make_unique<LinxEventFd>();
    auto queue = std::make_unique<LinxQueue>(std::move(efd), queueSize);

    LINX_INFO("Created UDP server: %s(%d), socket: %s:%d", serverId.c_str(), socket->getFd(), multicastIp.c_str(), port);
    return std::make_shared<UdpServer>(serverId, socket, std::move(queue));
}

std::shared_ptr<UdpServer> createServer(uint16_t port, size_t queueSize) {
    std::string ip = "0.0.0.0";

    auto socket = std::make_shared<UdpSocket>();
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open UDP socket for server on port: %d", port);
        return nullptr;
    }
    if (socket->bind(port) < 0) {
        LINX_ERROR("Failed to bind UDP socket for server on port: %d", port);
        return nullptr;
    }

    std::string serverId = ip + ":" + std::to_string(port);
    auto efd = std::make_unique<LinxEventFd>();
    auto queue = std::make_unique<LinxQueue>(std::move(efd), queueSize);

    LINX_INFO("Created UDP server: %s(%d), socket: %s:%d", serverId.c_str(), socket->getFd(), ip.c_str(), port);
    return std::make_shared<UdpServer>(serverId, socket, std::move(queue));
}

std::shared_ptr<UdpClient> createClient(const std::string &ip, uint16_t port) {

    auto socket = std::make_shared<UdpSocket>();
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open UDP socket for client: %s:%d", ip.c_str(), port);
        return nullptr;
    }
    if (isMulticastIp(ip)) {
        if (socket->setMulticastTtl(1) < 0) {
            LINX_ERROR("Failed to set multicast TTL for: %s", ip.c_str());
            return nullptr;
        }
    }
    if (isBroadcastIp(ip)) {
        if (socket->setBroadcast(true) < 0) {
            LINX_ERROR("Failed to set broadcast for: %s", ip.c_str());
            return nullptr;
        }
    }

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 65535);
    std::string clientId = "client_" + std::to_string(dis(gen)) + "_" + ip + ":" + std::to_string(port);

    LINX_INFO("Created UDP client: %s(%d) -> server socket: %s:%d", clientId.c_str(), socket->getFd(), ip.c_str(), port);
    return std::make_shared<UdpClient>(clientId, socket, PortInfo(ip, port));
}

bool isBroadcastIp(const std::string &ip) {
    return ip == "255.255.255.255";
}

bool isMulticastIp(const std::string &ip) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
        return false;
    }
    uint32_t ip_num = ntohl(addr.s_addr);
    return (ip_num >= 0xE0000000 && ip_num <= 0xEFFFFFFF);
}

} // namespace UdpFactory

// Explicit template instantiation
template class GenericServer<UdpSocket>;
template class GenericClient<UdpSocket>;