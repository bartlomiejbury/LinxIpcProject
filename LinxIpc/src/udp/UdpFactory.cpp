#include <sstream>
#include <random>
#include "UdpSocket.h"
#include "LinxEventFd.h"
#include "LinxQueue.h"
#include "LinxTrace.h"
#include "GenericServer.tpp"
#include "GenericClient.tpp"

namespace UdpFactory {

std::shared_ptr<UdpServer> createServer(uint16_t port, bool useMulticast, size_t queueSize) {
    std::string ip = useMulticast ? LINX_MULTICAST_IP_ADDRESS : "0.0.0.0";

    auto socket = std::make_shared<UdpSocket>();
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open UDP socket for server on port: %d", port);
        return nullptr;
    }
    if (socket->bind(port) < 0) {
        LINX_ERROR("Failed to bind UDP socket for server on port: %d", port);
        return nullptr;
    }

    if (isMulticastIp(ip)) {
        if (socket->joinMulticastGroup(ip) < 0) {
            LINX_ERROR("Failed to join multicast group: %s", ip.c_str());
            return nullptr;
        }
    }

    // Add argument for service name
    std::string serviceName = ip + ":" + std::to_string(port);
    auto efd = std::make_unique<LinxEventFd>();
    auto queue = std::make_unique<LinxQueue>(std::move(efd), queueSize);

    LINX_INFO("Created UDP server: %s(%d), socket: %s:%d", serviceName.c_str(), socket->getFd(), ip.c_str(), port);
    return std::make_shared<UdpServer>(serviceName, socket, std::move(queue));
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
    std::string instanceId = "client_" + std::to_string(dis(gen)) + "_" + ip + ":" + std::to_string(port);

    LINX_INFO("Created UDP client: %s(%d) -> server socket: %s:%d", instanceId.c_str(), socket->getFd(), ip.c_str(), port);
    return std::make_shared<UdpClient>(instanceId, socket, PortInfo(ip, port));
}

} // namespace UdpFactory

// Explicit template instantiation
template class GenericServer<UdpSocket>;
template class GenericClient<UdpSocket>;