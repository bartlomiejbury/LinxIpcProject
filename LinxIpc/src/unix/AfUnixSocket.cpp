#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "AfUnixSocket.h"
#include "LinxIpc.h"
#include "LinxTrace.h"

AfUnixSocket::AfUnixSocket(const std::string &socketName) {
    this->socketName = socketName;
}

AfUnixSocket::~AfUnixSocket() {
    this->close();
}

int AfUnixSocket::open() {
    if (this->fd >= 0) {
        LINX_INFO("IPC socket already connected for IPC");
        return -1;
    }

    if ((this->fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        this->fd = -1;
        LINX_ERROR("Cannot open IPC socket");
        return -1;
    }

    socklen_t address_length = createAddress(&this->address, socketName);

    if (bind(this->fd, (const struct sockaddr *)&this->address, address_length) < 0) {
        ::close(this->fd);
        this->fd = -1;
        LINX_ERROR("Cannot bind IPC socket");
        return -1;
    }

    return 0;
}

void AfUnixSocket::close() {
    if (this->fd >= 0) {
        ::shutdown(this->fd, SHUT_RDWR);
        ::close(this->fd);
        this->fd = -1;
    }
}

int AfUnixSocket::receive(RawMessagePtr *msg, std::unique_ptr<IIdentifier> *from, int timeoutMs) {

    if (this->fd < 0) {
        LINX_ERROR("IPC recv on wrong IPC socket");
        return -1;
    }

    struct pollfd fds[1];
    fds[0].fd = this->fd;
    fds[0].events = POLLIN;

    int pollrc = poll(fds, 1, timeoutMs);
    if (pollrc < 0) {
        if (errno == EBADF) {
            LINX_DEBUG("IPC recv socket closed IPC socket");
            return 0;
        } else {
            LINX_ERROR("IPC recv error IPC socket, errno: %d", errno);
            return -2;
        }
    } else if (pollrc == 0) {
        LINX_DEBUG("IPC recv timeout IPC socket");
        return 0;
    }

    int bytes_available = 0;
    ioctl(this->fd, FIONREAD, &bytes_available);
    std::vector<uint8_t> buffer(bytes_available);

    struct sockaddr_un client_address;
    socklen_t address_length = sizeof(struct sockaddr_un);
    memset(&client_address, 0, address_length);

    ssize_t len = recvfrom(this->fd, buffer.data(), bytes_available, 0,
                        (struct sockaddr *)&client_address, &address_length);
    if (len < 0) {
        if (errno == EBADF) {
            LINX_DEBUG("IPC recv socket closed IPC socket");
            return 0;
        } else {
            LINX_ERROR("IPC recv error IPC socket, errno: %d", errno);
            return -4;
        }
    }
    if (len != bytes_available) {
        LINX_ERROR("IPC recv wrong size: %d for IPC socket", len);
        return -5;
    }

    auto ipc = RawMessage::deserialize(std::move(buffer));
    if (ipc == nullptr) {
        LINX_ERROR("IPC recv deserialize failed for IPC socket");
        return -6;
    }

    if (from) {
        *from = std::move(std::make_unique<UnixInfo>(&client_address.sun_path[1]));
    }

    if (msg) {
        *msg = std::move(ipc);
    }

    return len;
}

int AfUnixSocket::send(const IMessage &message, const UnixInfo &to) {

    if (this->fd < 0) {
        LINX_ERROR("IPC send on wrong IPC socket");
        return -1;
    }

    uint32_t sendSize = message.getSize();
    uint8_t buffer[sendSize];
    uint32_t result = message.serialize(buffer, sendSize);

    if (result == 0) {
        LINX_ERROR("IPC send serialize error IPC socket, actual: %d, size: %d", result, sendSize);
        return -2;
    }

    struct sockaddr_un address {};
    socklen_t address_length = createAddress(&address, to.getValue());

    ssize_t len = sendto(this->fd, buffer, result, 0, (struct sockaddr *)&address, address_length);

    if (len < 0) {
        LINX_ERROR("IPC send error IPC socket, errno: %d", errno);
        return -3;
    }

    if ((uint32_t)len != result) {
        LINX_ERROR("IPC send wrong size: %d for IPC socket", len);
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
        LINX_ERROR("IPC flush on wrong IPC socket");
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

int AfUnixSocket::getFd() const {
    return fd;
}
