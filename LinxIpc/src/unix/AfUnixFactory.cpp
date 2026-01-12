#include <sstream>
#include <random>
#include "AfUnixSocket.h"
#include "LinxEventFd.h"
#include "LinxQueue.h"
#include "GenericSimpleServer.tpp"
#include "GenericServer.tpp"
#include "GenericClient.tpp"

namespace AfUnixFactory {

std::shared_ptr<AfUnixSimpleServer> createSimpleServer(const std::string &socketName) {
    auto socket = std::make_shared<AfUnixSocket>(socketName);
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open AF_UNIX socket for server: %s", socketName.c_str());
        return nullptr;
    }

    LINX_INFO("Created AF_UNIX server: %s(%d), socket: %s", socketName.c_str(), socket->getFd(), socketName.c_str());
    return std::make_shared<AfUnixSimpleServer>(socketName, socket);
}

std::shared_ptr<AfUnixServer> createServer(const std::string &socketName, size_t queueSize) {
    auto socket = std::make_shared<AfUnixSocket>(socketName);
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open AF_UNIX socket for server: %s", socketName.c_str());
        return nullptr;
    }

    auto efd = std::make_unique<LinxEventFd>();
    auto queue = std::make_unique<LinxQueue>(std::move(efd), queueSize);

    LINX_INFO("Created AF_UNIX worker server: %s(%d), socket: %s", socketName.c_str(), socket->getFd(), socketName.c_str());
    return std::make_shared<AfUnixServer>(socketName, socket, std::move(queue));
}

std::shared_ptr<AfUnixClient> createClient(const std::string &serverSocket) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 65535);
    std::string clientId = "client_" + std::to_string(dis(gen)) + "_" + serverSocket;

    auto socket = std::make_shared<AfUnixSocket>(clientId);
    if (socket->open() < 0) {
        LINX_ERROR("Failed to open AF_UNIX socket for client: %s", clientId.c_str());
        return nullptr;
    }

    LINX_INFO("Created AF_UNIX client: %s(%d) -> server socket: %s", clientId.c_str(), socket->getFd(), serverSocket.c_str());
    return std::make_shared<AfUnixClient>(clientId, socket, UnixInfo(serverSocket));
}

} // namespace AfUnixFactory

// Explicit template instantiation
template class GenericSimpleServer<AfUnixSocket>;
template class GenericServer<AfUnixSocket>;
template class GenericClient<AfUnixSocket>;
