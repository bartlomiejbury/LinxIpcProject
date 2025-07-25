#pragma once

class LinxIpcClient {
  public:
    virtual ~LinxIpcClient() {}

    virtual int send(const LinxMessageIpc &message) = 0;
    virtual LinxMessageIpcPtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) = 0;
    virtual LinxMessageIpcPtr sendReceive(const LinxMessageIpc &message, int timeoutMs = INFINITE_TIMEOUT, const std::vector<uint32_t> &sigsel = LINX_ANY_SIG) = 0;

    virtual std::string getName() const = 0;

    virtual bool connect(int timeout) = 0;

    virtual bool operator==(const LinxIpcClient &to) const = 0;
    virtual bool operator!=(const LinxIpcClient &to) const = 0;
};
