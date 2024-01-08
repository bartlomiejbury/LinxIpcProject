#pragma once

#include <pthread.h>
#include "LinxIpcEndpoint.h"
#include "LinxIpc.h"

class LinxIpcSocket;
class LinxQueue;

class LinxIpcEndpointThreadImpl : virtual public LinxIpcEndpoint {

  public:
    LinxIpcEndpointThreadImpl(const LinxIpcEndpointPtr &endpoint, LinxQueue *queue);
    ~LinxIpcEndpointThreadImpl();

    void start() override;
    void stop() override;

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;

    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    int getPollFd() const override;

  private:
    LinxQueue *queue;
    LinxIpcEndpointPtr endpoint;
    pthread_t threadId;
    bool running = false;

    void task();
    static void *threadFunc(void *arg);
};
