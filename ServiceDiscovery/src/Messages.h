#pragma once

#define DISCOVERY_REQ    IPC_SIG_BASE + 0x01
#define DISCOVERY_RSP    IPC_SIG_BASE + 0x02
#define REGISTER_REQ     IPC_SIG_BASE + 0x03
#define REGISTER_RSP     IPC_SIG_BASE + 0x04
#define UNREGISTER_REQ   IPC_SIG_BASE + 0x05
#define UNREGISTER_RSP   IPC_SIG_BASE + 0x06

struct DiscoveryUnregisterReqReq {
    char serviceName[20];
};

struct DiscoveryRsp {
    char ip[16];
    uint16_t port;
};

struct RegisterReq {
    char serviceName[20];
    uint16_t port;
    bool fromSource;
};

struct RegisterUnregisterRsp {
    bool success;
};

class DiscoveryRequestMessage : public IMessage {
  public:
    DiscoveryRequestMessage(const std::string &serviceName)
        : IMessage(DISCOVERY_REQ) {
        this->serviceName = serviceName;
    }

    uint32_t getPayloadSize() const override {
        return sizeof(DiscoveryUnregisterReqReq);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        DiscoveryUnregisterReqReq *data = reinterpret_cast<DiscoveryUnregisterReqReq*>(buffer);
        std::strncpy(data->serviceName, serviceName.c_str(), sizeof(data->serviceName) - 1);
        return this->getPayloadSize();
    }

    static std::unique_ptr<DiscoveryRequestMessage> fromRawMessage(const RawMessage &rawMsg) {
        DiscoveryUnregisterReqReq data = *rawMsg.getPayloadAs<DiscoveryUnregisterReqReq>();
        return std::make_unique<DiscoveryRequestMessage>(data.serviceName);
    }

    std::string serviceName;
};

class DiscoveryResponseMessage : public IMessage {
  public:
    DiscoveryResponseMessage(const std::string &ip, uint16_t port)
        : IMessage(DISCOVERY_RSP) {
        this->portInfo = PortInfo(ip, port);
    }

    uint32_t getPayloadSize() const override {
        return sizeof(DiscoveryRsp);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        DiscoveryRsp *data = reinterpret_cast<DiscoveryRsp*>(buffer);
        std::strncpy(data->ip, portInfo.ip.c_str(), sizeof(data->ip) - 1);
        data->port = htons(portInfo.port);
        return this->getPayloadSize();
    }

    static std::unique_ptr<DiscoveryResponseMessage> fromRawMessage(const RawMessage &rawMsg) {
        DiscoveryRsp data = *rawMsg.getPayloadAs<DiscoveryRsp>();
        std::string ip(data.ip);
        uint16_t port = ntohs(data.port);
        return std::make_unique<DiscoveryResponseMessage>(ip, port);
    }

    PortInfo portInfo;
};

class RegisterRequestMessage : public IMessage {
  public:
    RegisterRequestMessage(const std::string &serviceName, uint16_t port)
        : IMessage(REGISTER_REQ) {
        this->serviceName = serviceName;
        this->port = port;
    }

    uint32_t getPayloadSize() const override {
        return sizeof(RegisterReq);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        RegisterReq *data = reinterpret_cast<RegisterReq*>(buffer);
        std::strncpy(data->serviceName, serviceName.c_str(), sizeof(data->serviceName) - 1);
        data->port = htons(port);
        data->fromSource = fromSource;

        return this->getPayloadSize();
    }

    static std::unique_ptr<RegisterRequestMessage> fromRawMessage(const RawMessage &rawMsg) {
        RegisterReq data = *rawMsg.getPayloadAs<RegisterReq>();
        return std::make_unique<RegisterRequestMessage>(data.serviceName, ntohs(data.port));
    }

    std::string serviceName;
    uint16_t port;
    bool fromSource = false;
};

class RegisterResponseMessage : public IMessage {
  public:
    RegisterResponseMessage(bool success)
        : IMessage(REGISTER_RSP) {
            this->success = success;
    }

    uint32_t getPayloadSize() const override {
        return sizeof(RegisterUnregisterRsp);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        RegisterUnregisterRsp *data = reinterpret_cast<RegisterUnregisterRsp*>(buffer);
        data->success = success;
        return this->getPayloadSize();
    }

    static std::unique_ptr<RegisterResponseMessage> fromRawMessage(const RawMessage &rawMsg) {
        RegisterUnregisterRsp data = *rawMsg.getPayloadAs<RegisterUnregisterRsp>();
        return std::make_unique<RegisterResponseMessage>(data.success);
    }

    bool success;
};

class UnregisterRequestMessage : public IMessage {
  public:
    UnregisterRequestMessage(const std::string &serviceName)
        : IMessage(UNREGISTER_REQ) {
        this->serviceName = serviceName;
    }

     uint32_t getPayloadSize() const override {
        return sizeof(DiscoveryUnregisterReqReq);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        DiscoveryUnregisterReqReq *data = reinterpret_cast<DiscoveryUnregisterReqReq*>(buffer);
        std::strncpy(data->serviceName, serviceName.c_str(), sizeof(data->serviceName) - 1);
        return this->getPayloadSize();
    }

    static std::unique_ptr<UnregisterRequestMessage> fromRawMessage(const RawMessage &rawMsg) {
        DiscoveryUnregisterReqReq data = *rawMsg.getPayloadAs<DiscoveryUnregisterReqReq>();
        return std::make_unique<UnregisterRequestMessage>(data.serviceName);
    }

    std::string serviceName;
};

class UnregisterResponseMessage : public IMessage {
  public:
    UnregisterResponseMessage(bool success)
        : IMessage(UNREGISTER_RSP) {
            this->success = success;
    }

    uint32_t getPayloadSize() const override {
        return sizeof(RegisterUnregisterRsp);
    }

    virtual uint32_t serializePayload(uint8_t *buffer, uint32_t bufferSize) const override {
        RegisterUnregisterRsp *data = reinterpret_cast<RegisterUnregisterRsp*>(buffer);
        data->success = success;
        return this->getPayloadSize();
    }

    static std::unique_ptr<UnregisterResponseMessage> fromRawMessage(const RawMessage &rawMsg) {
        RegisterUnregisterRsp data = *rawMsg.getPayloadAs<RegisterUnregisterRsp>();
        return std::make_unique<UnregisterResponseMessage>(data.success);
    }

    bool success;
};
