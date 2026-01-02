#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include "LinxIpc.h"
#include "GenericSocket.h"
#include "AfUnix.h"

class AfUnixSocket : public GenericSocket<StringIdentifier> {
  public:
    AfUnixSocket(const std::string &socketName);
    virtual ~AfUnixSocket();

    virtual int getFd() const;

    virtual int send(const IMessage &message, const Identifier &to);
    virtual int receive(RawMessagePtr *msg, std::unique_ptr<IIdentifier> *from, int timeoutMs);

    virtual int flush();
    virtual int open();
    virtual void close();

  protected:
    int fd = -1;
    struct sockaddr_un address {};
    std::string socketName;

    socklen_t createAddress(struct sockaddr_un *address, const std::string &socketName);
};
