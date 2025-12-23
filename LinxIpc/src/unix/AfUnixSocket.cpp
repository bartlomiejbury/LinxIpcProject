#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "AfUnixSocket.h"
#include "LinxIpc.h"
#include "LinxTrace.h"

AfUnixSocket::AfUnixSocket(const std::string &serviceName) {
    this->serviceName = serviceName;
}

AfUnixSocket::~AfUnixSocket() {
    this->close();
}

bool AfUnixSocket::open() {
    if (this->fd >= 0) {
        LINX_INFO("IPC socket already connected for IPC: %s", serviceName.c_str());
        return true;
    }

    if ((this->fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        this->fd = -1;
        LINX_ERROR("Cannot open IPC socket for %s", serviceName.c_str());
        return false;
    }

    socklen_t address_length = createAddress(&this->address, serviceName);

    if (bind(this->fd, (const struct sockaddr *)&this->address, address_length) < 0) {
        ::close(this->fd);
        this->fd = -1;
        LINX_ERROR("Cannot bind IPC socket for %s", serviceName.c_str());
        return false;
    }

    LINX_INFO("Setup IPC: %s, fd: %d", serviceName.c_str(), this->fd);
    return true;
}

void AfUnixSocket::close() {
    if (this->fd >= 0) {
        LINX_DEBUG("closing fd: %d for IPC: %s", this->fd, this->serviceName.c_str());
        ::close(this->fd);
        this->fd = -1;
    }
}

int AfUnixSocket::receive(LinxMessagePtr *msg, std::string *from, int timeoutMs) {

    if (this->fd < 0) {
        LINX_ERROR("IPC recv on wrong socket for IPC: %s", this->serviceName.c_str());
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
            LINX_ERROR("IPC recv error IPC: %s, errno: %d", this->serviceName.c_str(), errno);
            return pollrc;
        } else if (pollrc == 0) {
            LINX_ERROR("IPC recv timeout IPC: %s", this->serviceName.c_str());
            return -2;
        }

        ioctl(this->fd, FIONREAD, &bytes_available);
    }

    std::vector<uint8_t> buffer(bytes_available);

    struct sockaddr_un client_address;
    socklen_t address_length = sizeof(struct sockaddr_un);
    memset(&client_address, 0, address_length);

    ssize_t len =
        recvfrom(this->fd, buffer.data(), bytes_available, 0, (struct sockaddr *)&client_address, &address_length);
    if (len < 0) {
        LINX_ERROR("IPC recv error IPC: %s, errno: %d", this->serviceName.c_str(), errno);
        return len;
    }
    if (len != bytes_available) {
        LINX_ERROR("IPC recv wrong size: %d for IPC: %s", len, this->serviceName.c_str());
        return -4;
    }

    auto ipc = LinxMessage::deserialize(buffer.data(), len);
    if (ipc == nullptr) {
        LINX_ERROR("IPC recv deserialize failed for IPC: %s", this->serviceName.c_str());
        return -1;
    }

    if (from) {
        *from = &client_address.sun_path[1];
    }

    auto payloadSize = ipc->getPayloadSize();
    if (msg) {
        *msg = std::move(ipc);
    }

    return payloadSize;
}

int AfUnixSocket::send(const LinxMessage &message, const std::string &to) {

    if (this->fd < 0) {
        LINX_ERROR("IPC send on wrong socket for IPC: %s", this->serviceName.c_str());
        return -1;
    }

    uint32_t sendSize = message.getSize();
    uint8_t buffer[sendSize];
    auto result = message.serialize(buffer, sendSize);

    if (!result.has_value()) {
        LINX_ERROR("IPC send serialize error IPC: %s, size: %d", this->serviceName.c_str(), sendSize);
        return -2;
    }

    struct sockaddr_un address {};
    socklen_t address_length = createAddress(&address, to);

    ssize_t len = sendto(this->fd, buffer, result.value(), 0, (struct sockaddr *)&address, address_length);

    if (len < 0) {
        LINX_ERROR("IPC send error IPC: %s(0x%x), errno: %d", this->serviceName.c_str(), message.getReqId(), errno);
        return -1 * errno;
    }

    if ((uint32_t)len != result.value()) {
        LINX_ERROR("IPC send wrong size: %d for IPC: ", len, this->serviceName.c_str());
        return -4;
    }

    return 0;
}

socklen_t AfUnixSocket::createAddress(struct sockaddr_un *address, const std::string &name) {
    socklen_t address_length = sizeof(address->sun_family) + name.size() + 1;
    address->sun_family = AF_UNIX;
    strncpy(&address->sun_path[1], name.data(), name.size());
    return address_length;
}

int AfUnixSocket::flush() {
    if (this->fd < 0) {
        LINX_ERROR("IPC recv on wrong socket for IPC: %s", this->serviceName.c_str());
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

std::string AfUnixSocket::getName() const {
    return this->serviceName;
}

int AfUnixSocket::getFd() const {
    return fd;
}
