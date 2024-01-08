#pragma once

#include <vector>

class LinxIpcClient {
  public:
    virtual ~LinxIpcClient() {}

    virtual int send(const LinxMessageIpc &message) = 0;
    virtual LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) = 0;
    virtual std::string getName() const = 0;

    virtual bool connect(int timeout) = 0;
};
