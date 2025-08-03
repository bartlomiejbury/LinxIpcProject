#pragma once

#include <map>
#include <thread>
#include "LinxIpc.h"
#include "LinxIpcSocket.h"

class LinxQueue;

class LinxIpcEndpointImpl : public std::enable_shared_from_this<LinxIpcEndpointImpl>, virtual public LinxIpcEndpoint {

  public:
    LinxIpcEndpointImpl(LinxIpcSocket *socket);
    ~LinxIpcEndpointImpl();

    int send(const LinxMessageIpc &message, const std::string &to) override;

    LinxMessageIpcPtr receive(int timeoutMs, 
                              const std::vector<uint32_t> &sigsel,
                              const std::optional<std::string> &from) override;

    int getPollFd() const override;
    std::string getName() const override;
    
  protected:
    LinxIpcSocket *socket;
};

LinxIpcEndpointPtr createLinxEndpoint(const std::string &serviceName);