#pragma once

#include <vector>
#include <optional>
#include <utility>

class LinxQueue {
   public:
    virtual ~LinxQueue() {}
    virtual int add(const LinxMessageIpcPtr &msg) = 0;
    virtual int size() const = 0;
    virtual int getFd() const = 0;
    virtual void clear() = 0;
    virtual LinxMessageIpcPtr get(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                                 const std::optional<LinxIpcClientPtr> &from) = 0;
};
