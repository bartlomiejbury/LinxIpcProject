#pragma once

#include "GenericServer.h"
#include "GenericClient.h"
#include <memory>

class AfUnixSocket;

// String-based identifier wrapper
class StringIdentifier : public IIdentifier {
  public:
    StringIdentifier() = default;
    explicit StringIdentifier(const std::string &value) : value(value) {}

    std::string format() const override {
        return value;
    }

    bool isEqual(const IIdentifier &other) const override {
        const auto *otherStr = dynamic_cast<const StringIdentifier*>(&other);
        return otherStr && value == otherStr->value;
    }

    std::unique_ptr<IIdentifier> clone() const override {
        return std::make_unique<StringIdentifier>(value);
    }

    bool operator==(const StringIdentifier &other) const {
        return value == other.value;
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
    using Identifier = StringIdentifier;
};


using AfUnixClient = GenericClient<AfUnixSocket>;
using AfUnixServer = GenericServer<AfUnixSocket>;

namespace AfUnixFactory {
    std::shared_ptr<AfUnixClient> createClient(const std::string &serviceName);
    std::shared_ptr<AfUnixServer> createServer(const std::string &serviceName, size_t queueSize = LINX_DEFAULT_QUEUE_SIZE);
}
