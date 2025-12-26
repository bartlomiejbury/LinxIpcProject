#include "RawMessage.h"
#include <arpa/inet.h>

ILinxMessage<uint8_t>::ILinxMessage(uint32_t reqId) : IMessage(reqId) {}

ILinxMessage<uint8_t>::ILinxMessage(uint32_t reqId, size_t payloadSize) : IMessage(reqId) {
    this->payload.resize(payloadSize);
}

ILinxMessage<uint8_t>::ILinxMessage(uint32_t reqId, const void *buffer, uint32_t payloadSize) : IMessage(reqId) {
    std::copy((uint8_t *)buffer, (uint8_t *)buffer + payloadSize, std::back_inserter(payload));
}

ILinxMessage<uint8_t>::ILinxMessage(uint32_t reqId, const std::vector<uint8_t> &buffer) : IMessage(reqId) {
    std::copy(buffer.begin(), buffer.end(), std::back_inserter(payload));
}

typedef struct {
    uint32_t reqId;
    uint8_t payload[0];
} IpcMessage;

std::unique_ptr<RawMessage> ILinxMessage<uint8_t>::deserialize(const uint8_t *buffer, uint32_t bufferSize) {
    if (bufferSize < sizeof(uint32_t)) {
        return nullptr;
    }
    const IpcMessage *ipc = (const IpcMessage *)buffer;
    uint32_t payloadSize = bufferSize - sizeof(ipc->reqId);
    uint32_t reqId = ntohl(ipc->reqId);
    return std::make_unique<RawMessage>(reqId, (void *)ipc->payload, payloadSize);
}
