#include <gmock/gmock.h>
#include "LinxIpc.h"

class LinxIpcEndpointMock : public LinxIpcEndpoint {
  public:
    MOCK_METHOD(int, send, (const LinxMessageIpc &message, const std::string &to));
    MOCK_METHOD(LinxMessageIpcPtr, receive, (int timeoutMs, 
                                             const std::vector<uint32_t> &sigsel,
                                             const std::optional<std::string> &from));
    MOCK_METHOD(int, getPollFd, (), (const));
    MOCK_METHOD(std::string, getName, (), (const));
};