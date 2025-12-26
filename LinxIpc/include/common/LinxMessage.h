#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <arpa/inet.h>

class IMessage {
  public:
    IMessage(uint32_t reqId) : reqId{reqId} {}
    virtual ~IMessage() = default;

    uint32_t getReqId() const {
        return reqId;
    }

    virtual uint32_t getPayloadSize() const = 0;
    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const = 0;

    uint32_t getSize() const {
        return sizeof(reqId) + getPayloadSize();
    }

    virtual uint32_t serialize(uint8_t *buffer, uint32_t bufferSize) const {
        uint32_t totalSize = this->getSize();
        if (bufferSize < totalSize) {
            return 0;
        }
        uint32_t netReqId = htonl(this->reqId);
        std::copy((uint8_t *)&netReqId, (uint8_t *)&netReqId + sizeof(netReqId), buffer);

        auto ret = serializePayload(buffer + sizeof(reqId), bufferSize - sizeof(reqId));
        return ret + sizeof(reqId) != totalSize ? 0 : totalSize;
    }

  protected:
    uint32_t reqId;
};

template<typename T = uint8_t>
class ILinxMessage : public IMessage {
  public:
    ILinxMessage(uint32_t reqId, const T& payload) : IMessage(reqId), payload(payload) {
    }
    virtual ~ILinxMessage() = default;
    virtual const T *getPayload() const {
        return &payload;
    }

    uint32_t getPayloadSize() const override {
        return sizeof(T);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        std::copy((uint8_t *)&this->payload, (uint8_t *)&this->payload + sizeof(this->payload), buffer);
        return this->getPayloadSize();
    }

    static std::unique_ptr<ILinxMessage<T>> deserialize(const uint8_t *buffer, uint32_t bufferSize) {
        if (bufferSize < sizeof(uint32_t) + sizeof(T)) {
            return nullptr;
        }
        const uint32_t *reqIdPtr = (const uint32_t *)buffer;
        uint32_t reqId = ntohl(*reqIdPtr);
        T payload;
        std::copy(buffer + sizeof(uint32_t), buffer + sizeof(uint32_t) + sizeof(T), (uint8_t *)&payload);
        return std::make_unique<ILinxMessage<T>>(reqId, payload);
    }

    T payload;
};
