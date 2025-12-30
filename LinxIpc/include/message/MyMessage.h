#pragma once

#include "LinxMessage.h"
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <optional>

class MyMessage : public IMessage {
    struct Data {
        int value;
        float temperature;
    };

  public:
    MyMessage(uint32_t reqId, int value, float temperature)
        : IMessage(reqId), value(value), temperature(temperature) {}

    ~MyMessage() = default;

    // Getters
    int getValue() const { return value; }
    float getTemperature() const { return temperature; }

    // Setters
    void setValue(int v) { value = v; }
    void setTemperature(float t) { temperature = t; }

    uint32_t getPayloadSize() const override {
        return sizeof(value) + sizeof(temperature);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        Data data = {
            .value = static_cast<int>(htonl(value)),
            .temperature = temperature
        };

        std::memcpy(buffer, &data, sizeof(data));
        return this->getPayloadSize();
    }

    static std::unique_ptr<MyMessage> fromRawMessage(const RawMessage &rawMsg) {
        Data data = *rawMsg.getPayloadAs<Data>();
        int value = static_cast<int>(ntohl(data.value));
        return std::make_unique<MyMessage>(rawMsg.getReqId(), value, data.temperature);
    }

  private:
    int value;
    float temperature;
};

using MyMessagePtr = std::unique_ptr<MyMessage>;
