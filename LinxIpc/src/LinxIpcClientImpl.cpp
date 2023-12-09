#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <time.h>
#include "trace.h"
#include "LinxIpcClientImpl.h"

LinxIpcClientImpl::LinxIpcClientImpl(LinxIpcEndpointPtr client, const std::string &serviceName) {

    this->client = client;
    this->serviceName = serviceName;

    LOG_INFO("Setup IPC Client: %s", serviceName.c_str());
}

int LinxIpcClientImpl::send(const LinxMessageIpc *message) {
    return client->send(message, shared_from_this());
}

LinxMessageIpcPtr LinxIpcClientImpl::receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel) {
    return client->receive(timeoutMs, sigsel, shared_from_this());
}

std::string LinxIpcClientImpl::getName() {
    return this->serviceName;
}

bool LinxIpcClientImpl::connect(int timeout) {
    if (timeout == IMMEDIATE_TIMEOUT) {
        return true;
    }

    return waitForConnect(timeout);
}

uint64_t LinxIpcClientImpl::getTimeMs() {

    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);

    uint64_t milisecondsFromSeconds = ts.tv_sec * MILLI_SECONDS;
    uint64_t milisecondsFromNanoseconds = ts.tv_nsec / (NANO_SECONDS / MILLI_SECONDS);
    return milisecondsFromSeconds + milisecondsFromNanoseconds;
}

bool LinxIpcClientImpl::waitForConnect(int timeoutMs) {

    static constexpr int pingInterval = 500;
    static constexpr int pingTimeout = 100;

    uint64_t startTime = getTimeMs();
    bool run = true;
    do {
        run = timeoutMs == INFINITE_TIMEOUT || (getTimeMs() - startTime) < (uint64_t)timeoutMs;

        LinxMessageIpc message{IPC_HUNT_REQ};
        int len = send(&message);
        if (len >= 0) {
            auto rsp = receive(pingTimeout, {IPC_HUNT_RSP});
            if (rsp != nullptr) {
                return true;
            }
        }
        usleep(pingInterval * MILLI_SECONDS);
    } while (run);

    LOG_ERROR("IPC Client: %s connection timed out", serviceName.c_str());
    return false;
}
