#pragma once

#include <memory>
#include <functional>
#include <stdint.h>
#include <string>

#define IPC_STATUS_MSG 1
#define IPC_STATUS_OK_MSG 1
#define IPC_STATUS_NOK_MSG 0
#define IPC_TIMEOUT 100
#define IPC_HUNT_REQ 1
#define IPC_HUNT_RSP 2

typedef enum {
  INFINITE_TIMEOUT = -1,
  IMMEDIATE_TIMEOUT = 0
} LinxTimeout;

const std::initializer_list<uint32_t> LINX_ANY_SIG({});

class LinxMessageIpc;
class LinxIpcClient;
class LinxIpcEndpoint;

using LinxMessageIpcPtr = std::shared_ptr<LinxMessageIpc>;
using LinxIpcClientPtr = std::shared_ptr<LinxIpcClient>;
using LinxIpcEndpointPtr = std::shared_ptr<LinxIpcEndpoint>;
using LinxIpcCallback = std::function<int(LinxMessageIpc *msg, void *data)>;   

LinxIpcEndpointPtr createLinxIpcEndpoint(const std::string &endpointName, int maxSize = 100);

#include "LinxIpcMessage.h"
#include "LinxIpcEndpoint.h"
#include "LinxIpcClient.h"
