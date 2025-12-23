#include "AfUnixEndpoint.h"
#include "AfUnixServer.h"
#include "Deadline.h"
#include "LinxTrace.h"

AfUnixEndpoint::AfUnixEndpoint(const std::shared_ptr<AfUnixSocket> &socket,
    const std::weak_ptr<AfUnixServer> &server,
    const std::string &serviceName): AfUnixClient(socket, serviceName) {
    this->server = server;
}

LinxMessagePtr AfUnixEndpoint::receive(int timeoutMs, const std::vector<uint32_t> &sigsel) {
    auto server = this->server.lock();
    if (!server) {
        LINX_ERROR("IPC Client: %s server is no longer available", serviceName.c_str());
        return nullptr;
    }
    auto msg = server->receive(timeoutMs, sigsel, shared_from_this());
    if (msg) {
        return std::move(msg->message);
    }
    return nullptr;
}

bool AfUnixEndpoint::isEqual(const LinxClient &other) const {
    const AfUnixEndpoint &otherClient = static_cast<const AfUnixEndpoint &>(other);
    return this->server.lock() == otherClient.server.lock() && AfUnixClient::isEqual(other);
}
