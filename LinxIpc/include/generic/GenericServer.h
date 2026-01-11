#pragma once

#include <memory>
#include <string>
#include <thread>
#include "LinxIpc.h"
#include "IIdentifier.h"

class LinxQueue;

template<typename SocketType>
class GenericServer: public LinxServer {
  public:
    using IdentifierType = typename SocketTraits<SocketType>::Identifier;

    GenericServer(const std::string &serverId,
                  const std::shared_ptr<SocketType> &socket,
                  std::unique_ptr<LinxQueue> &&queue);
    virtual ~GenericServer();

    LinxReceivedMessageSharedPtr receive(int timeoutMs = INFINITE_TIMEOUT,
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const IIdentifier *from = LINX_ANY_FROM);

    int getPollFd() const;
    bool start();
    void stop();
    int send(const IMessage &message, const IIdentifier &to);
    std::string getName() const override;

  protected:
    std::string serverId;
    std::shared_ptr<SocketType> socket;
    std::unique_ptr<LinxQueue> queue;
    std::thread workerThread;

    LinxReceivedMessagePtr receiveFromSocket(int timeoutMs,
              const std::vector<uint32_t> &sigsel,
              const IIdentifier *identifier);

    LinxReceivedMessageSharedPtr receiveFromTask(int timeoutMs,
              const std::vector<uint32_t> &sigsel,
              const IIdentifier *identifier);

    void task();
};
