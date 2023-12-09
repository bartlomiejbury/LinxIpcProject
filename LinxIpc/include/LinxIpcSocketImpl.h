#pragma once

#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include "LinxIpcSocket.h"

class LinxIpcSocketImpl: public LinxIpcSocket {
  public:
    LinxIpcSocketImpl(const std::string &serviceName);
    ~LinxIpcSocketImpl();

    std::string getName() override;
    int getFd() override;

    int send(const LinxMessageIpc *message, const std::string &to) override;
    int receive(LinxMessageIpc **msg, std::string *from, int timeout) override;

    int flush() override;

  protected:
    int fd = -1;
    struct sockaddr_un address {};
    std::string serviceName;

    socklen_t createAddress(struct sockaddr_un *address, const std::string &serviceName);
};
