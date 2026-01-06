#include <sstream>
#include <random>
#include "AfUnixSocket.h"
#include "LinxEventFd.h"
#include "LinxQueue.h"
#include "GenericServer.tpp"
#include "GenericClient.tpp"

namespace AfUnixFactory {

std::shared_ptr<AfUnixServer> createServer(const std::string &serviceName, size_t queueSize) {
    auto socket = std::make_shared<AfUnixSocket>(serviceName);
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open AF_UNIX socket for server: %s", serviceName.c_str());
        return nullptr;
    }

    auto efd = std::make_unique<LinxEventFd>();
    auto queue = std::make_unique<LinxQueue>(std::move(efd), queueSize);

    LINX_INFO("Created AF_UNIX server: %s(%d), socket: %s", serviceName.c_str(), socket->getFd(), serviceName.c_str());
    return std::make_shared<AfUnixServer>(serviceName, socket, std::move(queue));
}

std::shared_ptr<AfUnixClient> createClient(const std::string &serviceName) {

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 65535);
    std::string instanceId = "client_" + std::to_string(dis(gen)) + "_" + serviceName;


    auto socket = std::make_shared<AfUnixSocket>(instanceId);
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open AF_UNIX socket for client: %s", instanceId.c_str());
        return nullptr;
    }

    LINX_INFO("Created AF_UNIX client: %s(%d) -> server socket: %s", instanceId.c_str(), socket->getFd(), serviceName.c_str());
    return std::make_shared<AfUnixClient>(instanceId, socket, UnixInfo(serviceName));
}

} // namespace AfUnixFactory

// Explicit template instantiation
template class GenericServer<AfUnixSocket>;
template class GenericClient<AfUnixSocket>;
