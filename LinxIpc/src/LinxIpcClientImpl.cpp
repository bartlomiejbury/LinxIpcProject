#include <cassert>
#include "LinxIpcClientImpl.h"
#include "LinxTime.h"
#include "LinxTrace.h"
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "LinxIpcPrivate.h"

LinxIpcClientImpl::LinxIpcClientImpl(const LinxIpcServerPtr &server, const std::string &serviceName) {
    assert(server);

    this->server = server;
    this->serviceName = serviceName;

    LINX_INFO("Setup IPC Client: %s", serviceName.c_str());
}

int LinxIpcClientImpl::send(const LinxMessageIpc &message) {
    return server->send(message, shared_from_this());
}

LinxMessageIpcPtr LinxIpcClientImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel) {
    return server->receive(timeoutMs, sigsel, shared_from_this());
}

LinxMessageIpcPtr LinxIpcClientImpl::sendReceive(const LinxMessageIpc &message, int timeoutMs, const std::vector<uint32_t> &sigsel) {
    if (send(message) < 0) {
        LINX_ERROR("IPC Client: %s send failed: %s", serviceName.c_str(), strerror(errno));
        return nullptr;
    }

    return receive(timeoutMs, sigsel);
}

std::string LinxIpcClientImpl::getName() const {
    return this->serviceName;
}

bool LinxIpcClientImpl::connect(int timeoutMs) {

    static constexpr int pingInterval = 500;
    static constexpr int pingTimeout = 100;

    uint64_t startTime = getTimeMs();
    bool run = true;
    do {
        LinxMessageIpc message{IPC_PING_REQ};
        int len = send(message);
        if (len >= 0) {
            auto rsp = receive(pingTimeout, {IPC_PING_RSP});
            if (rsp != nullptr) {
                LINX_INFO("IPC Client: %s connected", serviceName.c_str());
                return true;
            }
        }
        usleep(pingInterval * MILLI_SECONDS);
        run = timeoutMs == INFINITE_TIMEOUT || (getTimeMs() - startTime) < (uint64_t)timeoutMs;
    } while (run);

    LINX_ERROR("IPC Client: %s connection timed out", serviceName.c_str());
    return false;
}

bool LinxIpcClientImpl::operator==(const LinxIpcClient &to) const {
    return this->getName() == to.getName();
}

bool LinxIpcClientImpl::operator!=(const LinxIpcClient &to) const {
    return ! (*this == to);
}