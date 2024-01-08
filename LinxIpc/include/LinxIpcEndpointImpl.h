#pragma once

#include <pthread.h>
#include "LinxIpcEndpoint.h"
#include "LinxIpc.h"

class LinxIpcSocket;
class LinxQueue;

class LinxIpcEndpointImpl : public std::enable_shared_from_this<LinxIpcEndpointImpl>, virtual public LinxIpcEndpoint {

  public:
    LinxIpcEndpointImpl(LinxIpcSocket *socket);
    ~LinxIpcEndpointImpl();

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                              const LinxIpcClientPtr &from = LINX_ANY_FROM) override;

    LinxIpcClientPtr createClient(const std::string &serviceName) override;
    int getPollFd() const override;

  private:
    LinxIpcSocket *socket;
};
