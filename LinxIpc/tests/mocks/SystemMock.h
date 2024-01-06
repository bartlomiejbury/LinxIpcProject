#include <time.h>
#include "CMock2.h"

class SystemMock : public CMockMocker<SystemMock> {
  public:
    MOCK_METHOD(int, clock_gettime, (clockid_t clk_id, struct timespec *tp));
    MOCK_METHOD(int, usleep, (uint64_t usec));

    MOCK_METHOD(ssize_t, read, (int fd, void *buf, size_t nbytes));
    MOCK_METHOD(ssize_t, write, (int fd, const void *buf, size_t n));
    MOCK_METHOD(int, close, (int fd));

    MOCK_METHOD(int, eventfd, (unsigned int count, int flags));
};
