#include <cassert>
#include <sstream>
#include <thread>
#include "AfUnixClient.h"
#include "AfUnixSocket.h"
#include "Deadline.h"
#include "LinxMessageIds.h"
#include "LinxTrace.h"

AfUnixClient::AfUnixClient(const std::shared_ptr<AfUnixSocket> &socket, const std::string &serviceName) {
    assert(socket);

    this->socket = socket;
    this->serviceName = serviceName;

    this->socket->open();
    LINX_INFO("Setup IPC Client: %s", serviceName.c_str());
}

int AfUnixClient::send(const LinxMessage &message) {
    return socket->send(message, this->serviceName);
}

LinxMessagePtr AfUnixClient::receive(int timeoutMs, const std::vector<uint32_t> &sigsel) {
    LinxMessagePtr msg{};
    std::string from;

    auto predicate = [&sigsel, this](LinxMessagePtr &msg, const std::string &from) {
        if (from == this->serviceName) {
            uint32_t reqId = msg->getReqId();
            return sigsel.size() == 0 || std::find_if(sigsel.begin(), sigsel.end(),
                    [reqId](uint32_t id) { return id == reqId; }) != sigsel.end();
        }
        return false;
    };

    auto deadline = Deadline(timeoutMs);
    do {
        int timeout = deadline.getRemainingTimeMs();
        int ret = socket->receive(&msg, &from, timeout);
        if (ret < 0) {
            LINX_ERROR("IPC Client: %s receive error: %s", serviceName.c_str(), strerror(errno));
            return nullptr;
        }
        if (predicate(msg, from)) {
            return msg;
        }
    } while (!deadline.isExpired());

    LINX_ERROR("IPC Client: %s receive timed out", serviceName.c_str());
    return nullptr;
}

LinxMessagePtr AfUnixClient::sendReceive(const LinxMessage &message, int timeoutMs, const std::vector<uint32_t> &sigsel) {
    if (send(message) < 0) {
        LINX_ERROR("IPC Client: %s send failed: %s", serviceName.c_str(), strerror(errno));
        return nullptr;
    }

    return receive(timeoutMs, sigsel);
}

bool AfUnixClient::connect(int timeoutMs) {

    static constexpr int pingTimeout = 100;
    Deadline deadline(timeoutMs);

    do {
        LinxMessage message{IPC_PING_REQ};
        int len = send(message);
        if (len >= 0) {
            auto rsp = receive(pingTimeout, {IPC_PING_RSP});
            if (rsp != nullptr) {
                LINX_INFO("IPC Client: %s connected", serviceName.c_str());
                return true;
            }
        }
    } while (!deadline.isExpired());

    LINX_ERROR("IPC Client: %s connection timed out", serviceName.c_str());
    return false;
}

bool AfUnixClient::isEqual(const LinxClient &other) const {
    const AfUnixClient &otherClient = static_cast<const AfUnixClient &>(other);
    return this->socket == otherClient.socket && this->serviceName == otherClient.serviceName;
}

std::string AfUnixClient::getName() const {
    return this->serviceName;
}

std::shared_ptr<AfUnixClient> AfUnixClient::create(const std::string &serviceName) {
    std::ostringstream oss;
    oss << serviceName << "client_" << std::this_thread::get_id() << "_" << serviceName;
    auto socketName = oss.str();

    auto socket = std::make_shared<AfUnixSocket>(socketName);
    return std::make_shared<AfUnixClient>(std::move(socket), serviceName);
}