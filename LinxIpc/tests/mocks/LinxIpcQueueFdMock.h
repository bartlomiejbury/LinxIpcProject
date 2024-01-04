#include "LinxIpc.h"
#include "LinxQueueFd.h"
#include "CMock2.h"

class LinxIpcQueueFdMock : public LinxQueueFd {
  public:
    MOCK_METHOD(int, getFd, (), (const));
    MOCK_METHOD(int, writeEvent, ());
    MOCK_METHOD(int, readEvent, ());
    MOCK_METHOD(void, clearEvents, ());
};
