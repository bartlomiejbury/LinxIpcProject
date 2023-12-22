#include "SystemMock.h"

CMOCK_MOCK_FUNCTION(SystemMock, int, clock_gettime, (clockid_t clk_id, struct timespec *tp));
