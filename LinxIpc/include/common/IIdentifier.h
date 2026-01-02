#pragma once

#include <string>
#include <memory>

// Interface for identifier types
class IIdentifier {
  public:
    virtual ~IIdentifier() = default;
    virtual std::string format() const = 0;

    virtual bool operator==(const IIdentifier &other) const {
      if (this == &other) {
          return true;
      }
      if (typeid(other) != typeid(*this)) {
          return false;
      }
      return isEqual(other);
    }
  protected:
    virtual bool isEqual(const IIdentifier &other) const = 0;
};

template<typename SocketType>
struct SocketTraits;