#pragma once

#include <memory>
#include <string>
#include <thread>
#include "LinxIpc.h"
#include "IIdentifier.h"
#include "GenericSimpleServer.h"

class LinxQueue;

template<typename IdentifierType>
class GenericServer: public GenericSimpleServer<IdentifierType> {
  public:

    GenericServer(const std::string &serverId,
                  const std::shared_ptr<GenericSocket<IdentifierType>> &socket,
                  std::unique_ptr<LinxQueue> &&queue);
    virtual ~GenericServer();

    LinxReceivedMessageSharedPtr receive(int timeoutMs = INFINITE_TIMEOUT,
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const IIdentifier *from = LINX_ANY_FROM) override;

    int getPollFd() const override;
    bool start() override;
    void stop() override;

  protected:
    std::unique_ptr<LinxQueue> queue;
    std::thread workerThread;

    void task();
};
