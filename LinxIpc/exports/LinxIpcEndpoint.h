#pragma once

class LinxIpcEndpoint {

  public:
    virtual ~LinxIpcEndpoint(){};

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) = 0;

    virtual LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                                      const LinxIpcClientPtr &from = LINX_ANY_FROM) = 0;
    virtual LinxIpcClientPtr createClient(const std::string &serviceName) = 0;
    virtual int getPollFd() const = 0;
};
