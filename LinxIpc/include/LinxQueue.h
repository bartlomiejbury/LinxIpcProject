#pragma once

#include <initializer_list>
#include "LinxIpc.h"

class LinxQueueContainer {
  public:
    LinxQueueContainer(const LinxMessageIpcPtr &msg, const std::string &from): message(msg), from(from) {
    }
    LinxMessageIpcPtr message;
    std::string from;
};

class LinxQueue {
  public:
    virtual ~LinxQueue() {}
    virtual int add(const LinxMessageIpcPtr &msg, const std::string &from) = 0;
    virtual int size() = 0;
    virtual void clear() = 0;
    virtual std::shared_ptr<LinxQueueContainer> get(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) = 0;
};
