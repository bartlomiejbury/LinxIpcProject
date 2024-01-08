#include "LinxIpc.h"
#include "CMock2.h"

class LinxIpcClientMock : public LinxIpcClient {
  public:
    MOCK_METHOD(int, send, (const LinxMessageIpc &message));
    MOCK_METHOD(LinxMessageIpcPtr, receive, (int timeoutMs, const std::vector<uint32_t> &sigsel));
    MOCK_METHOD(std::string, getName, (), (const));
    MOCK_METHOD(bool, connect, (int timeout));
};
