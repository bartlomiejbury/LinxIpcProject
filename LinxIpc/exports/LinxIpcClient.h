#pragma once

class LinxIpcClient {
  public:
    virtual ~LinxIpcClient() {}

    virtual int send(const LinxMessageIpc &message) = 0;
    virtual LinxMessageIpcPtr receive(int timeoutMs, const std::initializer_list<uint32_t> &sigsel) = 0;
    virtual std::string getName() const = 0;

    virtual bool connect(int timeout) = 0;
};
