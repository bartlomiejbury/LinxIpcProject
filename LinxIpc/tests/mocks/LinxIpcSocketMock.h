#include "LinxIpc.h"
#include "LinxIpcSocket.h"
#include "CMock2.h"

class LinxIpcSocketMock : public LinxIpcSocket {
  public:
    MOCK_METHOD(int, send, (const LinxMessageIpc &message, const std::string &to));
    MOCK_METHOD(int, receive, (LinxMessageIpcPtr * msg, std::string *from, int timeout));
    MOCK_METHOD(int, flush, ());

    MOCK_METHOD(std::string, getName, (), (const));
    MOCK_METHOD(int, getFd, (), (const));
};
