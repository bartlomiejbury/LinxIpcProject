#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include "LinxIpc.h"

class AfUnixSocket {
  public:
    AfUnixSocket(const std::string &serviceName);
    virtual ~AfUnixSocket();

    virtual std::string getName() const;
    virtual int getFd() const;

    virtual int send(const LinxMessage &message, const std::string &to);
    virtual int receive(LinxMessagePtr *msg, std::string *from, int timeout);

    virtual int flush();
    virtual bool open();
    virtual void close();

  protected:
    int fd = -1;
    struct sockaddr_un address {};
    std::string serviceName;

    socklen_t createAddress(struct sockaddr_un *address, const std::string &serviceName);
};
