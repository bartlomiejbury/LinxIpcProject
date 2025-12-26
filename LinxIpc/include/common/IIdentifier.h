#pragma once

#include <string>
#include <memory>

// Interface for identifier types
class IIdentifier {
  public:
    virtual ~IIdentifier() = default;
    virtual std::string format() const = 0;
    virtual bool isEqual(const IIdentifier &other) const = 0;
    virtual std::unique_ptr<IIdentifier> clone() const = 0;
};

template<typename SocketType>
struct SocketTraits;