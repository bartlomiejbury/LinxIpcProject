#pragma once

#include <map>
#include "LinxIpc.h"

class LinxIpcServerImpl : public LinxIpcServer {

  public:
    LinxIpcServerImpl(const LinxIpcEndpointPtr &endpoint);

    int handleMessage(int timeoutMs) override;
    void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) override;

    void start() {
        endpoint->start();
    }

    void stop() {
        endpoint->stop();
    }

    LinxIpcClientPtr createClient(const std::string &serviceName) {
        return endpoint->createClient(serviceName);
    }

    int getPollFd() const {
        return endpoint->getPollFd();
    }

  private:
    struct IpcContainer {
        LinxIpcCallback callback;
        void *data;
    };

    LinxIpcEndpointPtr endpoint;
    std::map<uint32_t, IpcContainer> handlers;
};
