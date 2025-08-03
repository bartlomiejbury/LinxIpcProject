#pragma once

#include <map>
#include "LinxIpc.h"

class LinxIpcHandler: public LinxIpcServer {
  public:
    virtual ~LinxIpcHandler(){};
    virtual int handleMessage(int timeoutMs = INFINITE_TIMEOUT) = 0;
};

struct IpcContainer {
    LinxIpcCallback callback;
    void *data;
};

typedef std::shared_ptr<LinxIpcHandler> LinxIpcHandlerPtr;
