#include "LinxIpc.h"
#include "LinxIpcEndpoint.h"
#include "CMock2.h"

class LinxIpcEndpointMock : public LinxIpcEndpoint {
  public:
    MOCK_METHOD(void, start, ());
    MOCK_METHOD(void, stop, ());
    MOCK_METHOD(int, send, (const LinxMessageIpc &message, const LinxIpcClientPtr &to));
    MOCK_METHOD(LinxMessageIpcPtr, receive, (int timeoutMs, const std::initializer_list<uint32_t> &sigsel));
    MOCK_METHOD(LinxMessageIpcPtr, receive,
                (int timeoutMs, const std::initializer_list<uint32_t> &sigsel, const LinxIpcClientPtr &from));
    MOCK_METHOD(LinxIpcClientPtr, createClient, (const std::string &serviceName));
    MOCK_METHOD(int, getPollFd, (), (const));
};
