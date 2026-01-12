#pragma once

#include <memory>
#include <string>
#include "LinxIpc.h"
#include "IIdentifier.h"

template<typename SocketType>
class GenericSimpleServer: public LinxServer {
  public:
    using IdentifierType = typename SocketTraits<SocketType>::Identifier;

    GenericSimpleServer(const std::string &serverId,
                  const std::shared_ptr<SocketType> &socket);
    virtual ~GenericSimpleServer();

    LinxReceivedMessageSharedPtr receive(int timeoutMs = INFINITE_TIMEOUT,
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const IIdentifier *from = LINX_ANY_FROM) override;

    int getPollFd() const override;
    bool start() override;
    void stop() override;
    int send(const IMessage &message, const IIdentifier &to) override;
    std::string getName() const override;

  protected:
    std::string serverId;
    std::shared_ptr<SocketType> socket;
};
