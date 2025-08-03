#pragma once

#include "LinxIpc.h"
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

class LinxIpcClientImpl : public std::enable_shared_from_this<LinxIpcClient>, public LinxIpcClient {
  public:
    LinxIpcClientImpl(const LinxIpcEndpointPtr &endpoint, const std::string &serviceName);
    ~LinxIpcClientImpl() {}
    int send(const LinxMessageIpc &message) override;
    LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) override;
    LinxMessageIpcPtr sendReceive(const LinxMessageIpc &message, int timeoutMs = INFINITE_TIMEOUT, const std::vector<uint32_t> &sigsel = LINX_ANY_SIG) override;

    std::string getName() const override;
    bool connect(int timeout) override;
    bool operator==(const LinxIpcClient &to) const override;
    bool operator!=(const LinxIpcClient &to) const override;

  protected:
    LinxIpcEndpointPtr endpoint;
    std::string serviceName;
};
