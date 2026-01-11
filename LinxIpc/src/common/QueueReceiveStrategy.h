#pragma once

#include "IReceiveStrategy.h"
#include "LinxIpc.h"
#include <memory>
#include <thread>
#include <string>

class LinxQueue;
class LinxServer;

/**
 * @brief Strategy for receiving messages from a queue (background thread mode)
 *
 * This strategy receives messages from a queue that is populated by a
 * background worker thread. Used when the server operates in asynchronous mode.
 */
template<typename SocketType>
class QueueReceiveStrategy : public IReceiveStrategy {
  public:
    QueueReceiveStrategy(std::unique_ptr<LinxQueue> &&queue,
                        const std::shared_ptr<SocketType> &socket);
    virtual ~QueueReceiveStrategy();

    LinxContainerPtr receive(
        int timeoutMs,
        const std::vector<uint32_t> &sigsel,
        const IIdentifier *from) override;

    int getPollFd() const override;

    bool start() override;
    void stop() override;

  private:
    std::unique_ptr<LinxQueue> queue;
    std::shared_ptr<SocketType> socket;
    std::thread workerThread;

    void task();
};
