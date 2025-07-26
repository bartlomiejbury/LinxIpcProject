#pragma once

#include <map>
#include <thread>
#include "LinxIpc.h"
#include "LinxIpcSocket.h"

class LinxQueue;

class LinxIpcSimpleServerImpl : public std::enable_shared_from_this<LinxIpcSimpleServerImpl>, virtual public LinxIpcServer {

  public:
    LinxIpcSimpleServerImpl(LinxIpcSocket *socket);
    ~LinxIpcSimpleServerImpl();

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;
    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    int getPollFd() const override;

    void start() override;
    void stop() override;

  protected:
    LinxIpcSocket *socket;
};

class LinxIpcExtendedServerImpl : public LinxIpcSimpleServerImpl {

  public:
    LinxIpcExtendedServerImpl(LinxIpcSocket *socket, LinxQueue *queue);
    ~LinxIpcExtendedServerImpl();

    LinxMessageIpcPtr receive(int timeoutMs = INFINITE_TIMEOUT, 
                              const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;
    int getPollFd() const override;

    void start() override;
    void stop() override;

  private:
    LinxQueue *queue;
    std::thread workerThread;

  protected:
    bool running = false;
    void task();
};

class LinxIpcHandlerImpl : public LinxIpcHandler {
  public:
    LinxIpcHandlerImpl(LinxIpcServerPtr server, std::map<uint32_t, IpcContainer> &handlers);
    ~LinxIpcHandlerImpl(){};

    int handleMessage(int timeoutMs) override;
    LinxIpcClientPtr createClient(const std::string &serviceName) override;

  private:
    LinxIpcServerPtr server;
    std::map<uint32_t, IpcContainer> handlers;
};

    
