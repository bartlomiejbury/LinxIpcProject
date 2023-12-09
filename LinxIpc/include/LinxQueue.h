#pragma once

#include <initializer_list>
#include "LinxIpc.h"

class LinxQueue {
  public:
    virtual ~LinxQueue() {}
    virtual int add(LinxMessageIpc *msg) = 0;
    virtual int size() = 0;
    virtual LinxMessageIpc *get(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) = 0;
};
