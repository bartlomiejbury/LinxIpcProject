#include <sys/eventfd.h>
#include <time.h>
#include "CMock2.h"

class SystemMock : public CMockMocker<SystemMock> {
  public:
    MOCK_METHOD(int, clock_gettime, (clockid_t clk_id, struct timespec *tp), (const));
    MOCK_METHOD(int, usleep, (uint64_t usec), (const));

    MOCK_METHOD(ssize_t, read, (int fd, void *buf, size_t nbytes), (const));
    MOCK_METHOD(ssize_t, write, (int fd, const void *buf, size_t n), (const));
    MOCK_METHOD(int, close, (int fd), (const));

    MOCK_METHOD(int, eventfd, (unsigned int count, int flags), (const));
};
