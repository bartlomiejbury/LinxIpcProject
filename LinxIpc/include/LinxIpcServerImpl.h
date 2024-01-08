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

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
        return endpoint->send(message, to);
    }

    LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                                      const LinxIpcClientPtr &from = LINX_ANY_FROM) {
        return endpoint->receive(timeoutMs, sigsel, from);
    }

  private:
    struct IpcContainer {
        LinxIpcCallback callback;
        void *data;
    };

    LinxIpcEndpointPtr endpoint;
    std::map<uint32_t, IpcContainer> handlers;
};
