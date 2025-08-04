#pragma once

#include <map>
#include <thread>
#include <atomic>
#include "LinxIpc.h"
#include "LinxIpcSocket.h"
#include "LinxIpcEndpointImpl.h"


class LinxQueue;

class LinxIpcServerImpl : public std::enable_shared_from_this<LinxIpcServerImpl>, public LinxIpcServer {

  public:
    LinxIpcServerImpl(LinxIpcSocket *socket, LinxQueue *queue);
    ~LinxIpcServerImpl();

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    LinxMessageIpcPtr receive(int timeoutMs = INFINITE_TIMEOUT, 
                              const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;
    int getPollFd() const override;

    void start() override;
    void stop() override;

  private:
    std::string serviceName;
    LinxIpcSocket *socket;
    LinxQueue *queue;
    std::thread workerThread;

  protected:
    std::atomic<bool> running = false;
    void task();
};
