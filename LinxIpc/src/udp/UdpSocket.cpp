#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "UdpSocket.h"
#include "LinxIpc.h"
#include "LinxTrace.h"


UdpSocket::UdpSocket() {
}

UdpSocket::~UdpSocket() {
    this->close();
}

int UdpSocket::getFd() const {
    return fd;
}

int UdpSocket::receive(RawMessagePtr *msg, PortInfo *from, int timeoutMs) {

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

    sockaddr_in client_address;
    socklen_t address_length = sizeof(sockaddr_in);
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
        *from = PortInfo{
            inet_ntoa(client_address.sin_addr),
            ntohs(client_address.sin_port)
        };
    }

    if (msg) {
        *msg = std::move(ipc);
    }

    return len;
}


int UdpSocket::send(const IMessage &message, const PortInfo &to) {
    if (this->fd < 0) {
        LINX_ERROR("IPC send on wrong IPC socket: %s:%d", to.ip.c_str(), to.port);
        return -1;
    }

    uint32_t sendSize = message.getSize();
    uint8_t buffer[sendSize];
    uint32_t result = message.serialize(buffer, sendSize);

    if (result == 0) {
        LINX_ERROR("IPC send serialize error IPC socket: %s:%d, actual: %d, size: %d", to.ip.c_str(), to.port, result, sendSize);
        return -2;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(to.port);
    if (inet_pton(AF_INET, to.ip.c_str(), &addr.sin_addr) != 1) {
        LINX_ERROR("IPC send invalid IP address IPC socket: %s:%d", to.ip.c_str(), to.port);
        return -3;
    }

    ssize_t len = sendto(this->fd, buffer, result, 0, (struct sockaddr *)&addr, sizeof(addr));

    if (len < 0) {
        LINX_ERROR("IPC send error IPC socket: %s:%d(0x%x), errno: %d", to.ip.c_str(), to.port, message.getReqId(), errno);
        return -4;
    }

    if ((uint32_t)len != result) {
        LINX_ERROR("IPC send wrong size: %d for IPC socket: %s:%d", len, to.ip.c_str(), to.port);
        return -5;
    }

    return 0;
}

int UdpSocket::flush() {

    if (this->fd < 0) {
        LINX_ERROR("IPC flush on wrong IPC socket");
        return -1;
    }

    int bytes_available = 0;
    ioctl(this->fd, FIONREAD, &bytes_available);
    if (bytes_available > 0) {
        std::vector<uint8_t> buffer(bytes_available);

        sockaddr_in client_address;
        socklen_t address_length = sizeof(sockaddr_in);
        memset(&client_address, 0, address_length);

        recvfrom(this->fd, buffer.data(), bytes_available, 0, (struct sockaddr *)&client_address, &address_length);
    }

    return bytes_available;
}

void UdpSocket::close() {
    if (this->fd >= 0) {
        ::shutdown(this->fd, SHUT_RDWR);
        ::close(this->fd);
        this->fd = -1;
    }
}

int UdpSocket::open() {
    if (this->fd >= 0) {
        LINX_ERROR("IPC open on already opened IPC socket");
        return -1;
    }

    if ((this->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        this->fd = -1;
        LINX_ERROR("Cannot open IPC socket");
        return -1;
    }

    return 0;
}

int UdpSocket::bind(uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    return ::bind(fd, (sockaddr*)&addr, sizeof(addr));
}

int UdpSocket::joinMulticastGroup(const std::string &multicastIp) {

    if (this->fd < 0) {
        LINX_ERROR("IPC joinMulticastGroup on wrong IPC socket");
        return -1;
    }

    LINX_INFO("Setting up multicast membership for IPC socket: %s", multicastIp.c_str());
    struct ip_mreq mreq{};
    mreq.imr_interface.s_addr = INADDR_ANY;
    inet_aton(multicastIp.c_str(), &mreq.imr_multiaddr);

    return setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

int UdpSocket::setBroadcast(bool enable) {

    if (this->fd < 0) {
        LINX_ERROR("IPC setBroadcast on wrong IPC socket");
        return -1;
    }

    LINX_INFO("Setting up broadcast for IPC socket");
    int enable_val = enable ? 1 : 0;
    return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &enable_val, sizeof(enable_val));
}

int UdpSocket::setMulticastTtl(int ttl) {

    if (this->fd < 0) {
        LINX_ERROR("IPC setMulticastTtl on wrong IPC socket");
        return -1;
    }

    int loop = 1;

    LINX_INFO("Setting up multicast for IPC socket");
    auto result1 = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    auto result2 = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    return result1 < 0 ? result1 : result2;
}

namespace UdpFactory {

bool isBroadcastIp(const std::string &ip) {
    return ip == "255.255.255.255";
}

bool isMulticastIp(const std::string &ip) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
        return false;
    }
    uint32_t ip_num = ntohl(addr.s_addr);
    return (ip_num >= 0xE0000000 && ip_num <= 0xEFFFFFFF);
}

}
