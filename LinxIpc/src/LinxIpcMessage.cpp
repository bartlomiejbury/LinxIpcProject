#include "LinxIpc.h"

LinxMessageIpc::LinxMessageIpc(uint32_t reqId) : reqId{reqId}, client{nullptr} {}

LinxMessageIpc::LinxMessageIpc(uint32_t reqId, void *buffer, uint32_t payloadSize, LinxIpcClient *client)
    : reqId{reqId}, client{client} {
    std::copy((uint8_t *)buffer, (uint8_t *)buffer + payloadSize, std::back_inserter(payload));
}

LinxMessageIpc::LinxMessageIpc() : reqId{0} {}

LinxMessageIpc::~LinxMessageIpc() {
    if (client) {
         delete client;
    }
};

LinxIpcClient *LinxMessageIpc::getClient() const {
     return this->client;
};

void LinxMessageIpc::setClient(LinxIpcClient *client) {
    this->client = client;
};

uint32_t LinxMessageIpc::getReqId() const {
    return this->reqId;
};

uint32_t LinxMessageIpc::getPayloadSize() const {
    return this->payload.size();
}

