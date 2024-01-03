#pragma once

#include <pthread.h>
#include "LinxIpcEndpoint.h"
#include "LinxIpc.h"

class LinxIpcSocket;
class LinxQueue;

class LinxIpcEndpointImpl : public std::enable_shared_from_this<LinxIpcEndpointImpl>, virtual public LinxIpcEndpoint {

  public:
    LinxIpcEndpointImpl(LinxQueue *queue, LinxIpcSocket *socket);
    ~LinxIpcEndpointImpl();

    void start() override;
    void stop() override;

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;

    LinxIpcClientPtr createClient(const std::string &serviceName) override;

    int getQueueSize() const override;
    int getPollFd() const override;

  private:
    LinxQueue *queue;
    LinxIpcSocket *socket;
    pthread_t threadId;
    bool running = false;

    void task();
    static void *threadFunc(void *arg);
};
