#pragma once

#include "IReceiveStrategy.h"
#include <memory>

class IIdentifier;
class LinxServer;

/**
 * @brief Strategy for receiving messages directly from a socket (synchronous mode)
 *
 * This strategy receives messages directly from the socket without queuing.
 * Used when the server operates in synchronous blocking mode.
 */
template<typename SocketType>
class DirectReceiveStrategy : public IReceiveStrategy {
  public:
    DirectReceiveStrategy(const std::shared_ptr<SocketType> &socket);
    virtual ~DirectReceiveStrategy() = default;

    LinxContainerPtr receive(
        int timeoutMs,
        const std::vector<uint32_t> &sigsel,
        const IIdentifier *from) override;

    int getPollFd() const override;

    bool start() override;
    void stop() override;

  private:
    std::shared_ptr<SocketType> socket;
};
