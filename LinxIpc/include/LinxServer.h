#pragma once

#include <functional>
#include <unordered_map>

struct LinxReceivedMessage;

using LinxReceivedMessagePtr = std::unique_ptr<LinxReceivedMessage>;
using LinxReceivedMessageSharedPtr = std::shared_ptr<LinxReceivedMessage>;
using LinxReceiveContextPtr = std::unique_ptr<LinxClient>;
using LinxIpcCallback = std::function<int(const LinxReceivedMessageSharedPtr &msg, void *data)>;

struct LinxReceivedMessage {
    LinxMessagePtr message;
    LinxReceiveContextPtr context;
};

class LinxServer {

  public:
    virtual ~LinxServer() = default;

    virtual LinxReceivedMessageSharedPtr receive(int timeoutMs = INFINITE_TIMEOUT,
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const LinxReceiveContextSharedPtr &from = LINX_ANY_FROM) = 0;

    virtual int getPollFd() const = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
};

struct IpcContainer {
    LinxIpcCallback callback;
    void *data;
};

class LinxIpcHandler: public LinxServer {
  public:
    LinxIpcHandler(const std::shared_ptr<LinxServer> &server);
    virtual ~LinxIpcHandler();

    virtual int handleMessage(int timeoutMs = INFINITE_TIMEOUT);
    LinxReceivedMessageSharedPtr receive(int timeoutMs = INFINITE_TIMEOUT,
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const LinxReceiveContextSharedPtr &from = LINX_ANY_FROM) override;

    bool start() override;
    void stop() override;
    int getPollFd() const override;

    LinxIpcHandler& registerCallback(uint32_t reqId, const LinxIpcCallback &callback, void *data = nullptr);

  private:
    std::shared_ptr<LinxServer> server;
    std::unordered_map<uint32_t, IpcContainer> handlers;
};
