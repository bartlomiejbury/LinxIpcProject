#pragma once

#include <vector>
#include <cstdint>
#include "LinxIpc.h"

class IIdentifier;

struct LinxContainer {
    RawMessagePtr message;
    std::unique_ptr<IIdentifier> from;
};

using LinxContainerPtr = std::shared_ptr<LinxContainer>;

class IReceiveStrategy {
  public:
    virtual ~IReceiveStrategy() = default;

    virtual LinxContainerPtr receive(
        int timeoutMs,
        const std::vector<uint32_t> &sigsel,
        const IIdentifier *from) = 0;

    virtual int getPollFd() const = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
};
