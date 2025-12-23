#include "LinxIpc.h"

LinxMessage::LinxMessage(uint32_t reqId) : reqId{reqId} {}

LinxMessage::LinxMessage(uint32_t reqId, size_t payloadSize) : reqId{reqId} {
    this->payload.resize(payloadSize);
}

LinxMessage::LinxMessage(uint32_t reqId, void *buffer, uint32_t payloadSize) : reqId{reqId} {
    std::copy((uint8_t *)buffer, (uint8_t *)buffer + payloadSize, std::back_inserter(payload));
}

LinxMessage::LinxMessage(uint32_t reqId, const std::vector<uint8_t> &buffer) : reqId{reqId} {
    std::copy(buffer.begin(), buffer.end(), std::back_inserter(payload));
}

uint32_t LinxMessage::getReqId() const {
    return this->reqId;
};

uint32_t LinxMessage::getPayloadSize() const {
    return this->payload.size();
}

uint32_t LinxMessage::getSize() const {
    return sizeof(this->reqId) + this->getPayloadSize();
}

std::optional<uint32_t> LinxMessage::serialize(uint8_t *buffer, uint32_t bufferSize) const {
    uint32_t totalSize = this->getSize();
    if (bufferSize < totalSize) {
        return {};
    }

    uint8_t *ptr = buffer;
    std::copy((uint8_t *)&this->reqId, (uint8_t *)&this->reqId + sizeof(this->reqId), ptr);
    ptr += sizeof(this->reqId);
    std::copy(this->payload.begin(), this->payload.end(), ptr);

    return totalSize;
}

typedef struct {
    uint32_t reqId;
    uint8_t payload[0];
} IpcMessage;

std::unique_ptr<LinxMessage> LinxMessage::deserialize(const uint8_t *buffer, uint32_t bufferSize) {
    if (bufferSize < sizeof(uint32_t)) {
        return nullptr;
    }
    const IpcMessage *ipc = (const IpcMessage *)buffer;
    uint32_t payloadSize = bufferSize - sizeof(ipc->reqId);
    return std::make_unique<LinxMessage>(ipc->reqId, (void *)ipc->payload, payloadSize);
}