#pragma once

#include "LinxMessage.h"
#include <vector>

// Template specialization for uint8_t (raw byte buffer)
template<>
class ILinxMessage<uint8_t> : public IMessage {
  public:
    ILinxMessage(uint32_t reqId);
    ILinxMessage(uint32_t reqId, size_t payloadSize);
    ILinxMessage(uint32_t reqId, const void *buffer, uint32_t payloadSize);
    ILinxMessage(uint32_t reqId, const std::vector<uint8_t> &buffer);

    virtual ~ILinxMessage() = default;

    const uint8_t *getPayload() const {
        return (const uint8_t *)payload.data();
    }

    template<typename T>
    const T *getPayloadAs() const {
        return reinterpret_cast<const T *>(payload.data());
    }

    uint32_t getPayloadSize() const override {
        return payload.size();
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        std::copy(this->payload.begin(), this->payload.end(), buffer);
        return this->getPayloadSize();
    }
    static std::unique_ptr<ILinxMessage<uint8_t>> deserialize(const uint8_t *buffer, uint32_t bufferSize);

  protected:
    std::vector<uint8_t> payload{};
};

using RawMessage = ILinxMessage<uint8_t>;
using RawMessagePtr = std::unique_ptr<RawMessage>;
