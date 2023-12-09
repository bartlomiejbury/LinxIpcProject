#pragma once

class LinxIpcEndpoint {

  public:
    virtual ~LinxIpcEndpoint() {};

    virtual int send(const LinxMessageIpc *message, LinxIpcClientPtr to) = 0;

    virtual LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel) = 0;
    virtual LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClientPtr from) = 0;

    virtual int receive() = 0;

    virtual void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) = 0;
    virtual LinxIpcClientPtr createClient(const std::string &serviceName) = 0;

    virtual int getQueueSize() = 0;
    virtual int getFd() = 0;
};
