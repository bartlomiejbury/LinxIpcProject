#pragma once

#include <vector>
#include <optional>
#include <utility>

using LinxQueueElement = std::shared_ptr<std::pair<LinxMessageIpcPtr, std::string>>;

class LinxQueue {
   public:
    virtual ~LinxQueue() {}
    virtual int add(const LinxMessageIpcPtr &msg, const std::string &clientName) = 0;
    virtual int size() const = 0;
    virtual int getFd() const = 0;
    virtual void clear() = 0;
    virtual LinxQueueElement get(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                                const std::optional<std::string> &from) = 0;
};
