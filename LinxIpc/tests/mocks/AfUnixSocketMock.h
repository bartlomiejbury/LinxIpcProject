#include <gmock/gmock.h>
#include "AfUnixSocket.h"
#include "LinxIpc.h"
#include "AfUnix.h"

class AfUnixSocketMock : public AfUnixSocket {
  public:
    AfUnixSocketMock() : AfUnixSocket("MOCK_SOCKET") {}
    ~AfUnixSocketMock() override = default;
    MOCK_METHOD(int, send, (const IMessage &message, const StringIdentifier &to));
    MOCK_METHOD(int, receive, (RawMessagePtr * msg, StringIdentifier *from, int timeout));
    MOCK_METHOD(int, flush, ());
    MOCK_METHOD(int, open, ());
    MOCK_METHOD(void, close, ());

    MOCK_METHOD(std::string, getName, (), (const));
    MOCK_METHOD(int, getFd, (), (const));
};
