#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include "LinxIpc.h"
#include "GenericSocket.h"
#include "UdpLinx.h"

class UdpSocket : public GenericSocket<PortInfo> {
  public:
    UdpSocket();
    virtual ~UdpSocket();

    virtual int getFd() const;

    virtual int send(const IMessage &message, const Identifier &to);
    virtual int receive(RawMessagePtr *msg, std::unique_ptr<IIdentifier> *from, int timeout);

    virtual int flush();
    virtual void close();

    virtual int open();
    virtual int bind(uint16_t port, const std::string &multicastIp = "0.0.0.0");
    virtual int joinMulticastGroup(const std::string &multicastIp);
    virtual int setBroadcast(bool enable);
    virtual int setMulticastTtl(int ttl);

  protected:
    int fd = -1;
};