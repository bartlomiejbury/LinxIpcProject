#pragma once

class LinxIpcServer {

  public:
    virtual ~LinxIpcServer(){};
    virtual int handleMessage(int timeoutMs = INFINITE_TIMEOUT) = 0;
    virtual void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) = 0;

    virtual LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                      const LinxIpcClientPtr &from = LINX_ANY_FROM) = 0;
    virtual LinxIpcClientPtr createClient(const std::string &serviceName) = 0;
    virtual int getPollFd() const = 0;

};
