#pragma once

class LinxMessageIpc {
  public:
    template <typename T>
    LinxMessageIpc(uint32_t reqId, const T &data) : LinxMessageIpc(reqId, (uint8_t *)&data, sizeof(T)) {}

    template <typename T = uint8_t>
    T *getPayload() const {
        return (T *)this->payload.data();
    }

    LinxMessageIpc(uint32_t reqId);
    LinxMessageIpc(uint32_t reqId, void *buffer, uint32_t payloadSize);
    LinxMessageIpc(uint32_t reqId, const std::vector<uint8_t> &buffer);
    LinxMessageIpc();

    ~LinxMessageIpc();
    LinxIpcClient *getClient() const;
    void setClient(const LinxIpcClientPtr &client);
    uint32_t getReqId() const;
    uint32_t getPayloadSize() const;

  private:
    uint32_t reqId;
    std::vector<uint8_t> payload{};
    LinxIpcClientPtr client = nullptr;
};

using LinxMessageIpcPtr = std::shared_ptr<LinxMessageIpc>;
