#include <time.h>
#include "CMock2.h"

class SystemMock : public CMockMocker<SystemMock> {
  public:
    MOCK_METHOD(int, clock_gettime, (clockid_t clk_id, struct timespec *tp));
    MOCK_METHOD(int, usleep, (uint64_t usec));
};
