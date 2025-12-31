#include <sys/eventfd.h>
#include <time.h>
#include <sys/socket.h>
#include "CMock2.h"

class SystemMock : public CMockMocker<SystemMock> {
  public:
    MOCK_METHOD(int, clock_gettime, (clockid_t clk_id, struct timespec *tp), (const));
    MOCK_METHOD(int, usleep, (uint64_t usec), (const));

    MOCK_METHOD(ssize_t, read, (int fd, void *buf, size_t nbytes), (const));
    MOCK_METHOD(ssize_t, write, (int fd, const void *buf, size_t n), (const));
    MOCK_METHOD(int, close, (int fd), (const));

    MOCK_METHOD(int, eventfd, (unsigned int count, int flags), (const));

    MOCK_METHOD(int, socket, (int domain, int type, int protocol), (const));
    MOCK_METHOD(int, bind, (int sockfd, const struct sockaddr *addr, socklen_t addrlen), (const));
    MOCK_METHOD(int, setsockopt, (int sockfd, int level, int optname,
                                 const void *optval, socklen_t optlen), (const));
    MOCK_METHOD(ssize_t, recvfrom, (int sockfd, void *buf, size_t len, int flags,
                                   struct sockaddr *src_addr, socklen_t *addrlen), (const));
    MOCK_METHOD(ssize_t, sendto, (int sockfd, const void *buf, size_t len, int flags,
                                 const struct sockaddr *dest_addr, socklen_t addrlen), (const));
};