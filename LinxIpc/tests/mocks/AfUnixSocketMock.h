#include <gmock/gmock.h>
#include "AfUnixSocket.h"
#include "LinxIpc.h"

class AfUnixSocketMock : public AfUnixSocket {
  public:
    AfUnixSocketMock() : AfUnixSocket("MOCK_SOCKET") {}
    ~AfUnixSocketMock() override = default;
    MOCK_METHOD(int, send, (const LinxMessage &message, const std::string &to));
    MOCK_METHOD(int, receive, (LinxMessagePtr * msg, std::string *from, int timeout));
    MOCK_METHOD(int, flush, ());
    MOCK_METHOD(bool, open, ());
    MOCK_METHOD(void, close, ());

    MOCK_METHOD(std::string, getName, (), (const));
    MOCK_METHOD(int, getFd, (), (const));
};
