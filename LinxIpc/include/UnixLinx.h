#pragma once

#include "GenericSimpleServer.h"
#include "GenericServer.h"
#include "GenericClient.h"
#include <memory>

class AfUnixSocket;

// String-based identifier wrapper
class UnixInfo : public IIdentifier {
  public:
    UnixInfo() = default;
    explicit UnixInfo(const std::string &value) : value(value) {}

    std::string format() const override {
        return value;
    }

    bool isEqual(const IIdentifier &other) const override {
        const auto *otherStr = dynamic_cast<const UnixInfo*>(&other);
        return value == otherStr->value;
    }

    const std::string& getValue() const {
        return value;
    }

  private:
    std::string value;
};

// Socket traits specialization
template<>
struct SocketTraits<AfUnixSocket> {
    using Identifier = UnixInfo;
};


using AfUnixClient = GenericClient<AfUnixSocket>;
using AfUnixSimpleServer = GenericSimpleServer<AfUnixSocket>;
using AfUnixServer = GenericServer<AfUnixSocket>;

namespace AfUnixFactory {
    std::shared_ptr<AfUnixClient> createClient(const std::string &serverSocket);
    std::shared_ptr<AfUnixSimpleServer> createSimpleServer(const std::string &socketName);
    std::shared_ptr<AfUnixServer> createServer(const std::string &socketName, size_t queueSize = LINX_DEFAULT_QUEUE_SIZE);
}
