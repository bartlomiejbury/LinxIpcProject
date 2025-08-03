#pragma once  

#include "LinxIpc.h"

class LinxIpcHandlerImpl: public LinxIpcHandler {
  public:
    LinxIpcHandlerImpl(const LinxIpcServerPtr &server, const std::map<uint32_t, IpcContainer> &handlers);
    ~LinxIpcHandlerImpl() override;
    
    int handleMessage(int timeoutMs = INFINITE_TIMEOUT) override;
    LinxIpcClientPtr createClient(const std::string &serviceName) override;

    int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) override;
    
    LinxMessageIpcPtr receive(int timeoutMs = INFINITE_TIMEOUT, 
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const LinxIpcClientPtr &from = LINX_ANY_FROM) override;

    void start() override;
    void stop() override;
    int getPollFd() const override;


  private:
    LinxIpcServerPtr server;
    std::map<uint32_t, IpcContainer> handlers;
};



    
