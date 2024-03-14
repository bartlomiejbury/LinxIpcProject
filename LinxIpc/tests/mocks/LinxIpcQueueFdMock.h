#include <gmock/gmock.h>
#include "LinxIpc.h"
#include "LinxQueueFd.h"

class LinxIpcQueueFdMock : public LinxQueueFd {
  public:
    MOCK_METHOD(int, getFd, (), (const));
    MOCK_METHOD(int, writeEvent, ());
    MOCK_METHOD(int, readEvent, ());
    MOCK_METHOD(void, clearEvents, ());
};
