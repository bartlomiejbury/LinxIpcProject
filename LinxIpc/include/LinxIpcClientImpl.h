#pragma once

#include "LinxIpc.h"
#include "LinxIpcEndpoint.h"
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

class LinxIpcClientImpl : public std::enable_shared_from_this<LinxIpcClient>, public LinxIpcClient {
  public:
    LinxIpcClientImpl(const LinxIpcEndpointPtr &client, const std::string &serviceName);
    ~LinxIpcClientImpl() {}
    int send(const LinxMessageIpc &message) override;
    LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) override;
    std::string getName() const override;
    bool connect(int timeout) override;

  protected:
    LinxIpcEndpointPtr client;
    std::string serviceName;
};
