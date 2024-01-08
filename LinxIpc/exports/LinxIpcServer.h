#pragma once

class LinxIpcServer: public virtual LinxIpcEndpoint {

  public:
    virtual ~LinxIpcServer(){};
    virtual int handleMessage(int timeoutMs = INFINITE_TIMEOUT) = 0;
    virtual void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) = 0;
};
