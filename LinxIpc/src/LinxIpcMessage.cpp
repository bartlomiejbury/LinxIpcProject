#include "LinxIpc.h"

LinxMessageIpc::LinxMessageIpc() : LinxMessageIpc(0) {}

LinxMessageIpc::LinxMessageIpc(uint32_t reqId) : reqId{reqId} {}

LinxMessageIpc::LinxMessageIpc(uint32_t reqId, void *buffer, uint32_t payloadSize)
    : reqId{reqId} {
    std::copy((uint8_t *)buffer, (uint8_t *)buffer + payloadSize, std::back_inserter(payload));
}

LinxMessageIpc::~LinxMessageIpc() {
};

LinxIpcClient *LinxMessageIpc::getClient() const {
     return this->client.get();
};

void LinxMessageIpc::setClient(const LinxIpcClientPtr &client) {
    this->client = client;
};

uint32_t LinxMessageIpc::getReqId() const {
    return this->reqId;
};

uint32_t LinxMessageIpc::getPayloadSize() const {
    return this->payload.size();
}

