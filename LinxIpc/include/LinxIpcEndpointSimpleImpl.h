#pragma once

#include <pthread.h>
#include "LinxIpcEndpoint.h"
#include "LinxIpc.h"

class LinxIpcSocket;
class LinxQueue;

class LinxIpcEndpointSimpleImpl : public std::enable_shared_from_this<LinxIpcEndpointSimpleImpl>, virtual public LinxIpcEndpoint {

  public:
    LinxIpcEndpointSimpleImpl(LinxIpcSocket *socket);
    ~LinxIpcEndpointSimpleImpl();

    void start() override;
    void stop() override;

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;

    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    int getPollFd() const override;

  private:
    LinxIpcSocket *socket;
};
