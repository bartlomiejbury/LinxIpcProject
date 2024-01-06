#include "SystemMock.h"

CMOCK_MOCK_FUNCTION(SystemMock, int, clock_gettime, (clockid_t clk_id, struct timespec *tp));
CMOCK_MOCK_FUNCTION(SystemMock, int, usleep, (uint64_t usec));
CMOCK_MOCK_FUNCTION(SystemMock, ssize_t, read, (int fd, void *buf, size_t nbytes));
CMOCK_MOCK_FUNCTION(SystemMock, ssize_t, write, (int fd, const void *buf, size_t n));
CMOCK_MOCK_FUNCTION(SystemMock, int, close, (int fd));
CMOCK_MOCK_FUNCTION(SystemMock, int, eventfd, (unsigned int count, int flags));
