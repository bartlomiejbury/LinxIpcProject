#pragma once

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>

class IIdentifier;

const inline uint32_t IPC_SIG_BASE = 0x10000000;
const inline size_t LINX_DEFAULT_QUEUE_SIZE = 100;
const inline int IMMEDIATE_TIMEOUT = 0;
const inline int INFINITE_TIMEOUT = -1;
const inline std::initializer_list<uint32_t> LINX_ANY_SIG({});
const inline IIdentifier *LINX_ANY_FROM = nullptr;

#include "LinxMessage.h"
#include "RawMessage.h"
#include "LinxClient.h"
#include "LinxServer.h"
#include "MyMessage.h"