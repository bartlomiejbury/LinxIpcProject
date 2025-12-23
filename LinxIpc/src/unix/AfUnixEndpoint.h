#pragma once

#include "AfUnixClient.h"
#include "LinxIpc.h"

class AfUnixServer;

class AfUnixEndpoint: public std::enable_shared_from_this<AfUnixEndpoint>, public AfUnixClient {
  public:
    AfUnixEndpoint(const std::shared_ptr<AfUnixSocket> &socket,
      const std::weak_ptr<AfUnixServer> &server,
      const std::string &serviceName);
    ~AfUnixEndpoint() = default;

    LinxMessagePtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) override;
    bool isEqual(const LinxClient &other) const override;

  protected:
    std::weak_ptr<AfUnixServer> server;
};
