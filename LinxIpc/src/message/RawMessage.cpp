#include "RawMessage.h"
#include <arpa/inet.h>
#include <cstring>

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

ILinxMessage<uint8_t>::ILinxMessage(uint32_t reqId, std::vector<uint8_t> &&buffer) : IMessage(reqId), payload(std::move(buffer)) {
    // Move the entire vector - zero copy!
}

std::unique_ptr<RawMessage> ILinxMessage<uint8_t>::deserialize(std::vector<uint8_t> &&buffer) {
    if (buffer.size() < sizeof(uint32_t)) {
        return nullptr;
    }

    // Extract reqId from the buffer
    uint32_t reqId;
    std::memcpy(&reqId, buffer.data(), sizeof(uint32_t));
    reqId = ntohl(reqId);

    // Remove the reqId bytes from the beginning, keeping only payload
    buffer.erase(buffer.begin(), buffer.begin() + sizeof(uint32_t));

    // Move the vector into the RawMessage - zero copy!
    return std::make_unique<RawMessage>(reqId, std::move(buffer));
}
