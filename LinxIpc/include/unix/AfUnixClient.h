#pragma once

#include "LinxIpc.h"

class AfUnixSocket;

class AfUnixClient: public LinxClient {
  public:
    AfUnixClient(const std::shared_ptr<AfUnixSocket> &socket, const std::string &serviceName);
    ~AfUnixClient() = default;

    int send(const LinxMessage &message) override;
    LinxMessagePtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) override;
    LinxMessagePtr sendReceive(const LinxMessage &message, int timeoutMs = INFINITE_TIMEOUT, const std::vector<uint32_t> &sigsel = LINX_ANY_SIG) override;

    bool connect(int timeout) override;
    bool isEqual(const LinxClient &other) const override;

    std::string getName() const override;
    static std::shared_ptr<AfUnixClient> create(const std::string &serviceName);

  protected:
    std::shared_ptr<AfUnixSocket> socket;
    std::string serviceName;
};
