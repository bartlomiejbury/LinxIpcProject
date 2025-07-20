#pragma once

#include <map>
#include <pthread.h>
#include "LinxIpc.h"
#include "LinxIpcSocket.h"

class LinxQueue;

struct IpcContainer {
    LinxIpcCallback callback;
    void *data;
};

class LinxIpcSimpleServerImpl : public std::enable_shared_from_this<LinxIpcSimpleServerImpl>, virtual public LinxIpcServer {

  public:
    LinxIpcSimpleServerImpl(LinxIpcSocket *socket);
    ~LinxIpcSimpleServerImpl();

    int handleMessage(int timeoutMs) override;
    void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) override;

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;
    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    int getPollFd() const override;

  protected:
    LinxIpcSocket *socket;

  private:
    std::map<uint32_t, IpcContainer> handlers;
};

class LinxIpcExtendedServerImpl : virtual public LinxIpcSimpleServerImpl, virtual public LinxIpcExtendedServer {

  public:
    LinxIpcExtendedServerImpl(LinxIpcSocket *socket, LinxQueue *queue);
    ~LinxIpcExtendedServerImpl();

    void start() override;
    void stop() override;

    LinxMessageIpcPtr receive(int timeoutMs = INFINITE_TIMEOUT, 
                              const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;
    int getPollFd() const override;

  private:
    LinxQueue *queue;
    pthread_t threadId;
    bool running = false;

    void task();
    static void *threadFunc(void *arg);
};
