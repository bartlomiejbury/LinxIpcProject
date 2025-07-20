#include <gmock/gmock.h>
#include "LinxIpc.h"
#include "LinxIpcServer.h"

class LinxIpcServerMock : public LinxIpcServer {
  public:
    MOCK_METHOD(int, send, (const LinxMessageIpc &message, const LinxIpcClientPtr &to));
    MOCK_METHOD(LinxMessageIpcPtr, receive, (int timeoutMs, const std::vector<uint32_t> &sigsel,
                                      const LinxIpcClientPtr &from));
    MOCK_METHOD(int, getPollFd, (), (const));

    MOCK_METHOD(LinxIpcClientPtr, createClient, (const std::string &serviceName));
    MOCK_METHOD(int, handleMessage, (int timeoutMs));
    MOCK_METHOD(void, registerCallback, (uint32_t reqId, LinxIpcCallback callback, void *data));

    MOCK_METHOD(void, start, ());
    MOCK_METHOD(void, stop, ());
};
