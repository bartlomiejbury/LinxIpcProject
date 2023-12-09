#pragma once

#include <map>
#include <pthread.h>
#include "LinxIpcEndpoint.h"
#include "LinxIpc.h"

class LinxIpcSocket;
class LinxQueue;

class LinxIpcEndpointImpl : public std::enable_shared_from_this<LinxIpcEndpointImpl>, public LinxIpcEndpoint {

  public:
    
    LinxIpcEndpointImpl(LinxQueue *queue, LinxIpcSocket *socket);
    ~LinxIpcEndpointImpl();

    int send(const LinxMessageIpc *message, LinxIpcClientPtr to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel) override;
    LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClientPtr from) override;

    int receive() override;

    void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) override;
    LinxIpcClientPtr createClient(const std::string &serviceName) override;

    int getQueueSize() override;
    int getFd() override;
    
  private:
    struct IpcContainer {
        LinxIpcCallback callback;
        void *data;
    };

    LinxQueue *queue;
    LinxIpcSocket *socket;
    std::map<uint32_t, IpcContainer> handlers;
    pthread_t threadId;
    bool running;

    void task();
    static void *threadFunc(void *arg);
};
