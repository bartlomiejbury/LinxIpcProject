#include <unistd.h>
#include <errno.h>
#include <vector>
#include <sys/ioctl.h>
#include <poll.h>
#include "LinxIpc.h"
#include "LinxIpcSocketImpl.h"
#include "trace.h"

typedef struct {
    uint32_t reqId;
    uint8_t payload[0];
} IpcMessage;

LinxIpcSocketImpl::LinxIpcSocketImpl(const std::string &serviceName) {

    this->serviceName = serviceName;

    if ((this->fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        this->fd = -1;
        LOG_ERROR("Cannot open IPC socket for %s", serviceName.c_str());
        return;
    }

    socklen_t address_length = createAddress(&this->address, serviceName);

    if (bind(this->fd, (const struct sockaddr *)&this->address, address_length) < 0) {
        close(this->fd);
        this->fd = -1;
        LOG_ERROR("Cannot bind IPC socket for %s", serviceName.c_str());
        return;
    }

    LOG_INFO("Setup IPC: %s, fd: %d", serviceName.c_str(), this->fd);
}

LinxIpcSocketImpl::~LinxIpcSocketImpl() {
    if (this->fd) {
        LOG_DEBUG("closing fd: %d for IPC: %s", this->fd, this->serviceName.c_str());
        close(this->fd);
    }
}

int LinxIpcSocketImpl::receive(LinxMessageIpcPtr *msg, std::string *from, int timeoutMs) {

    if (this->fd < 0) {
        LOG_ERROR("IPC recv on wrong socket for IPC: %s", this->serviceName.c_str());
        return -1;
    }

    int bytes_available = 0;
    ioctl(this->fd, FIONREAD, &bytes_available);
    if (bytes_available == 0) {

        struct pollfd fds[1];
        fds[0].fd = this->fd;
        fds[0].events = POLLIN;

        int pollrc = poll(fds, 1, timeoutMs);
        if (pollrc < 0) {
            LOG_ERROR("IPC recv error IPC: %s, errono: %d", this->serviceName.c_str(), errno);
            return pollrc;
        } else if (pollrc == 0) {
            LOG_ERROR("IPC recv timeout IPC: %s", this->serviceName.c_str());
            return -1;
        }

        ioctl(this->fd, FIONREAD, &bytes_available);
    }

    std::vector<uint8_t> buffer(bytes_available);
    IpcMessage *ipc = (IpcMessage *)buffer.data();

    struct sockaddr_un client_address;
    socklen_t address_length = sizeof(struct sockaddr_un);
    memset(&client_address, 0, address_length);

    ssize_t len =
        recvfrom(this->fd, buffer.data(), bytes_available, 0, (struct sockaddr *)&client_address, &address_length);
    if (len < 0) {
        LOG_ERROR("IPC recv error IPC: %s, errono: %d", this->serviceName.c_str(), errno);
        return len;
    }

    if ((uint32_t)len < sizeof(ipc->reqId)) {
        LOG_ERROR("IPC recv wrong size: %d for IPC: ", len, this->serviceName.c_str());
        return -1;
    }

    if (from) {
        *from = &client_address.sun_path[1];
    }

    if (msg) {
        *msg = std::make_shared<LinxMessageIpc>(ipc->reqId, ipc->payload, len - sizeof(ipc->reqId));
    }

    return len - sizeof(ipc->reqId);
}

int LinxIpcSocketImpl::send(const LinxMessageIpc *message, const std::string &to) {

    if (this->fd < 0) {
        LOG_ERROR("IPC send on wrong socket for IPC: %s", this->serviceName.c_str());
        return -1;
    }

    uint32_t payloadSize = message->getPayloadSize();
    uint8_t *msgData = message->getPayload<uint8_t>();

    uint32_t sendSize = payloadSize + sizeof(uint32_t);
    uint8_t buffer[sendSize];
    IpcMessage *ipc = (IpcMessage *)buffer;
    ipc->reqId = message->getReqId();

    memcpy(ipc->payload, msgData, payloadSize);

    struct sockaddr_un address {};
    socklen_t address_length = createAddress(&address, to);

    ssize_t len = sendto(this->fd, buffer, sendSize, 0, (struct sockaddr *)&address, address_length);

    if (len < 0) {
        LOG_ERROR("IPC send error IPC: %s(%d), errono: %d", this->serviceName.c_str(), ipc->reqId, errno);
        return -1;
    }

    if ((uint32_t)len != sendSize) {
        LOG_ERROR("IPC send wrong size: %d for IPC: ", len, this->serviceName.c_str());
        return -1;
    }

    return 0;
}

socklen_t LinxIpcSocketImpl::createAddress(struct sockaddr_un *address, const std::string &name) {
    socklen_t address_length = sizeof(address->sun_family) + name.size() + 1;
    address->sun_family = AF_UNIX;
    strncpy(&address->sun_path[1], name.data(), name.size());
    return address_length;
}

int LinxIpcSocketImpl::flush() {
    if (this->fd < 0) {
        LOG_ERROR("IPC recv on wrong socket for IPC: %s", this->serviceName.c_str());
        return -1;
    }

    int bytes_available = 0;
    ioctl(this->fd, FIONREAD, &bytes_available);
    if (bytes_available > 0) {
        std::vector<uint8_t> buffer(bytes_available);

        struct sockaddr_un client_address;
        socklen_t address_length = sizeof(struct sockaddr_un);
        memset(&client_address, 0, address_length);

        recvfrom(this->fd, buffer.data(), bytes_available, 0, (struct sockaddr *)&client_address, &address_length);
    }

    return bytes_available;
}

std::string LinxIpcSocketImpl::getName() const {
    return this->serviceName;
}

int LinxIpcSocketImpl::getFd() const {
    return fd;
}
