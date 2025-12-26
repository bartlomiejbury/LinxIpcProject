#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include "LinxIpc.h"
#include "IIdentifier.h"

template<typename SocketType>
class GenericClient : public LinxClient {
  public:
    using IdentifierType = typename SocketTraits<SocketType>::Identifier;
    using PredicateType = std::function<bool(RawMessagePtr&, const std::vector<uint32_t>&, const IdentifierType&)>;

    GenericClient(const std::string &instanceId,
                  const std::shared_ptr<SocketType> &socket,
                  const IdentifierType &identifier);
    virtual ~GenericClient();

    int send(const IMessage &message) override;
    RawMessagePtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) override;
    RawMessagePtr sendReceive(const IMessage &message, int timeoutMs = INFINITE_TIMEOUT,
                               const std::vector<uint32_t> &sigsel = LINX_ANY_SIG) override;
    bool connect(int timeoutMs) override;
    bool isEqual(const LinxClient &other) const override;
    std::string getName() const override;

  protected:
    std::string instanceId;
    std::shared_ptr<SocketType> socket;
    IdentifierType identifier;
};
