#pragma once

#include <optional>
#include <vector>

class LinxMessage {
  public:
    template <typename T>
    LinxMessage(uint32_t reqId, const T &data) : LinxMessage(reqId, (uint8_t *)&data, sizeof(T)) {}

    LinxMessage(uint32_t reqId);
    LinxMessage(uint32_t reqId, size_t payloadSize);
    LinxMessage(uint32_t reqId, void *buffer, uint32_t payloadSize);
    LinxMessage(uint32_t reqId, const std::vector<uint8_t> &buffer);

    ~LinxMessage() = default;

    template <typename T = uint8_t>
    T *getPayload() const {
        return (T *)this->payload.data();
    }

    uint32_t getReqId() const;
    uint32_t getPayloadSize() const;
    uint32_t getSize() const;
    std::optional<uint32_t> serialize(uint8_t *buffer, uint32_t bufferSize) const;

    static std::unique_ptr<LinxMessage> deserialize(const uint8_t *buffer, uint32_t bufferSize);

  private:
    uint32_t reqId;
    std::vector<uint8_t> payload{};
};

using LinxMessagePtr = std::unique_ptr<LinxMessage>;
