#include <gmock/gmock.h>
#include "LinxIpc.h"

class LinxServerMock : public LinxServer {
  public:
    MOCK_METHOD(LinxReceivedMessageSharedPtr, receive, (int timeoutMs,
                                             const std::vector<uint32_t> &sigsel,
                                             const LinxReceiveContextSharedPtr &from));

    MOCK_METHOD(int, getPollFd, (), (const));
    MOCK_METHOD(bool, start, ());
    MOCK_METHOD(void, stop, ());
};
