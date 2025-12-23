#pragma once

#include <atomic>
#include <thread>
#include "LinxIpc.h"

class LinxQueue;
class AfUnixSocket;

class AfUnixServer : public std::enable_shared_from_this<AfUnixServer>, public LinxServer {

  public:
    AfUnixServer(const std::shared_ptr<AfUnixSocket> &socket, std::unique_ptr<LinxQueue> &&queue, const std::string &serviceName);
    ~AfUnixServer();

    LinxReceivedMessageSharedPtr receive(int timeoutMs = INFINITE_TIMEOUT,
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const LinxReceiveContextSharedPtr &from = LINX_ANY_FROM) override;

    int getPollFd() const override;
    bool start() override;
    void stop() override;

    LinxReceiveContextSharedPtr createContext(const std::string &from) const;

    static std::shared_ptr<AfUnixServer> create(const std::string &serviceName, size_t queueSize = LINX_DEFAULT_QUEUE_SIZE);

  private:
    std::string serviceName;
    std::shared_ptr<AfUnixSocket> socket;
    std::unique_ptr<LinxQueue> queue;
    std::thread workerThread;

  protected:
    std::atomic<bool> running = false;
    void task();
};
