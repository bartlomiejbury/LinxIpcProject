#include "LinxIpc.h"
#include "CMock2.h"

class LinxIpcEndpointMock : public LinxIpcClient {
  public:
    MOCK_METHOD(int, send, (const LinxMessageIpc *message, const LinxIpcClientPtr &to));
    MOCK_METHOD(LinxMessageIpcPtr, receive, (int timeoutMs, const std::initializer_list<uint32_t> &sigsel));
    MOCK_METHOD(LinxMessageIpcPtr, receive,
                (int timeoutMs, const std::initializer_list<uint32_t> &sigsel, const LinxIpcClientPtr &from));
    MOCK_METHOD(int, receive, ());
    MOCK_METHOD(void, registerCallback, (uint32_t reqId, LinxIpcCallback callback, void *data));
    MOCK_METHOD(LinxIpcClientPtr, createClient, (const std::string &serviceName));
    MOCK_METHOD(int, getQueueSize, (), (const));
    MOCK_METHOD(int, getFd, (), (const));
};
