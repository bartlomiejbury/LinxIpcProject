#pragma once

#include <map>
#include <pthread.h>
#include "LinxIpc.h"
#include "LinxIpcEndpoint.h"

class LinxQueue;

class LinxIpcServerImpl : virtual public LinxIpcServer {

  public:
    LinxIpcServerImpl(const LinxIpcEndpointPtr &endpoint, LinxQueue *queue);
    ~LinxIpcServerImpl();

    void start() override;
    void stop() override;

    int handleMessage(int timeoutMs) override;
    void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) override;

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;

    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    int getPollFd() const override;

  private:
    LinxQueue *queue;
    LinxIpcEndpointPtr endpoint;
    pthread_t threadId;
    bool running = false;

    struct IpcContainer {
        LinxIpcCallback callback;
        void *data;
    };

    std::map<uint32_t, IpcContainer> handlers;

    void task();
    static void *threadFunc(void *arg);
};
