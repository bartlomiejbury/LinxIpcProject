#pragma once

#include <map>

class LinxIpcServer {

  public:
    virtual ~LinxIpcServer(){};

    virtual int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) = 0;
    virtual LinxMessageIpcPtr receive(int timeoutMs = INFINITE_TIMEOUT, 
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const LinxIpcClientPtr &from = LINX_ANY_FROM) = 0;
    

    virtual LinxIpcClientPtr createClient(const std::string &serviceName) = 0;
    virtual int getPollFd() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

typedef std::shared_ptr<LinxIpcServer> LinxIpcServerPtr;
