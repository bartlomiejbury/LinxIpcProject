#pragma once

#include <memory>
#include <functional>
#include <cstdint>
#include <string>
#include <vector>
#include <initializer_list>

#define IPC_HUNT_REQ 1
#define IPC_HUNT_RSP 2

class LinxMessageIpc;
class LinxIpcClient;
class LinxIpcServer;
class LinxIpcExtendedServer;

using LinxMessageIpcPtr = std::shared_ptr<LinxMessageIpc>;
using LinxIpcClientPtr = std::shared_ptr<LinxIpcClient>;
using LinxIpcServerPtr = std::shared_ptr<LinxIpcServer>;
using LinxIpcExtendedServerPtr = std::shared_ptr<LinxIpcExtendedServer>;
using LinxIpcCallback = std::function<int(LinxMessageIpc *msg, void *data)>;

const int IMMEDIATE_TIMEOUT = 0;
const int INFINITE_TIMEOUT = -1;
const std::initializer_list<uint32_t> LINX_ANY_SIG({});
const LinxIpcClientPtr LINX_ANY_FROM = nullptr;

#include "LinxIpcMessage.h"
#include "LinxIpcClient.h"
#include "LinxIpcServer.h"

LinxIpcServerPtr createLinxIpcSimpleServer(const std::string &endpointName);
LinxIpcExtendedServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize = 100);
