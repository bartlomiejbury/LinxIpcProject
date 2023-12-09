#pragma once

#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include "LinxIpc.h"

class LinxIpcClientImpl : public std::enable_shared_from_this<LinxIpcClient>, public LinxIpcClient {
  public:
    LinxIpcClientImpl(LinxIpcEndpointPtr client, const std::string &serviceName);
    ~LinxIpcClientImpl() {}
    int send(const LinxMessageIpc *message) override;
    LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel) override;
    std::string getName() override;
    bool connect(int timeout) override;

  protected:
    LinxIpcEndpointPtr client;
    std::string serviceName;

  private:
    bool waitForConnect(int timeoutMs);
    uint64_t getTimeMs();
};
