#pragma once

#include <memory>
#include <functional>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <initializer_list>

#define IPC_SIG_BASE 100

class LinxMessageIpc;
class LinxIpcClient;

using LinxMessageIpcPtr = std::shared_ptr<LinxMessageIpc>;
using LinxIpcClientPtr = std::shared_ptr<LinxIpcClient>;
using LinxIpcCallback = std::function<int(LinxMessageIpc *msg, void *data)>;

const int IMMEDIATE_TIMEOUT = 0;
const int INFINITE_TIMEOUT = -1;
const std::initializer_list<uint32_t> LINX_ANY_SIG({});
const LinxIpcClientPtr LINX_ANY_FROM = nullptr;

#include "LinxIpcMessage.h"
#include "LinxIpcClient.h"
#include "LinxIpcEndpoint.h"
#include "LinxIpcServer.h"
#include "LinxIpcHandler.h"

class LinxIpcHandlerBuilder {
  public:
    LinxIpcHandlerBuilder(const std::string &serverName) : serverName(serverName) {}
    LinxIpcHandlerBuilder& registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data);
    LinxIpcHandlerPtr build();
  
  protected:
    std::string serverName;
    std::map<uint32_t, IpcContainer> handlers;
};

LinxIpcServerPtr createIpcServer(const std::string &serviceName, int maxSize = 100);
LinxIpcClientPtr createIpcClient(const std::string &serviceName);