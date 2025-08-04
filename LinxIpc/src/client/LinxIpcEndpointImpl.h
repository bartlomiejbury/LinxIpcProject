#pragma once

#include <map>
#include <thread>
#include "LinxIpc.h"
#include "LinxIpcSocketImpl.h"

class LinxQueue;

class LinxIpcEndpointImpl : public std::enable_shared_from_this<LinxIpcEndpointImpl>, virtual public LinxIpcServer {

  public:
    LinxIpcEndpointImpl(LinxIpcSocket *socket);
    ~LinxIpcEndpointImpl();

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    LinxMessageIpcPtr receive(int timeoutMs = INFINITE_TIMEOUT, 
                              const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;

    int getPollFd() const override;
    void start() override;
    void stop() override;

  protected:
    LinxIpcSocket *socket;
};

typedef std::shared_ptr<LinxIpcEndpointImpl> LinxIpcEndpointPtr;