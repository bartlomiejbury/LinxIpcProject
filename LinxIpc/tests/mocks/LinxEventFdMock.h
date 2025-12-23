#include <gmock/gmock.h>
#include "LinxEventFd.h"
#include "LinxIpc.h"

class LinxEventFdMock : public LinxEventFd {
  public:
    MOCK_METHOD(int, getFd, (), (const));
    MOCK_METHOD(int, writeEvent, ());
    MOCK_METHOD(int, readEvent, ());
    MOCK_METHOD(void, clearEvents, ());
};
