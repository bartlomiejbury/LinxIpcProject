#pragma once

#include <map>
#include "LinxIpc.h"
#include "LinxIpcEndpointImpl.h"

class LinxIpcServerImpl : public LinxIpcEndpointImpl, public LinxIpcServer {

  public:
    LinxIpcServerImpl(LinxQueue *queue, LinxIpcSocket *socket);

    int handleMessage(int timeoutMs) override;
    void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) override;

  private:
    struct IpcContainer {
        LinxIpcCallback callback;
        void *data;
    };

    std::map<uint32_t, IpcContainer> handlers;
};
