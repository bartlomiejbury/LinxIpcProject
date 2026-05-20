#pragma once

#include <cstring>
#include "Deadline.h"
#include "LinxTrace.h"
#include "LinxMessageIds.h"
#include "GenericSocket.h"
#include "LinxMessageFilter.h"

template<typename IdentifierType>
GenericClient<IdentifierType>::GenericClient(const std::string &clientId,
                                             const std::shared_ptr<GenericSocket<IdentifierType>> &socket,
                                             const IdentifierType &identifier)
    : clientId(clientId), socket(socket), identifier(identifier) {
}

template<typename IdentifierType>
GenericClient<IdentifierType>::~GenericClient() {
    LINX_INFO("[%s] Stopping", getName().c_str());
    this->socket->close();
}

template<typename IdentifierType>
int GenericClient<IdentifierType>::send(const IMessage &message) {
    LINX_DEBUG("[%s] Sending message reqId: 0x%x",
            getName().c_str(), message.getReqId());
    auto ret = socket->send(message, identifier);
    if (ret < 0) {
        LINX_ERROR("[%s] Send error: %d", getName().c_str(), ret);
    }
    return ret;
}

template<typename IdentifierType>
RawMessagePtr GenericClient<IdentifierType>::receive(int timeoutMs, const std::vector<uint32_t> &sigsel) {
    RawMessagePtr msg{};
    std::unique_ptr<IIdentifier> from;

    auto predicate = [this, &sigsel](const RawMessagePtr &msg, const std::unique_ptr<IIdentifier> &from) {
        return LinxMessageFilter::matchesFrom(from.get(), &identifier) &&
               LinxMessageFilter::matchesSignalSelector(*msg, sigsel);
    };

    auto deadline = Deadline(timeoutMs);
    do {
        int timeout = deadline.getRemainingTimeMs();
        int ret = socket->receive(&msg, &from, timeout);
        if (ret == 0) {
            LINX_DEBUG("[%s] receive timeout", getName().c_str());
            return nullptr;
        }
        if (ret < 0) {
            LINX_ERROR("[%s] receive error: %d", getName().c_str(), ret);
            return nullptr;
        }
        if (predicate(msg, from)) {
            return msg;
        }
    } while (!deadline.isExpired());

    LINX_ERROR("[%s] receive timed out", getName().c_str());
    return nullptr;
}

template<typename IdentifierType>
RawMessagePtr GenericClient<IdentifierType>::sendReceive(const IMessage &message, int timeoutMs,
                                                                        const std::vector<uint32_t> &sigsel) {
    if (send(message) < 0) {
        return nullptr;
    }
    return receive(timeoutMs, sigsel);
}

template<typename IdentifierType>
bool GenericClient<IdentifierType>::connect(int timeoutMs) {
    static constexpr int pingTimeout = 100;
    Deadline deadline(timeoutMs);

    do {
        RawMessage message{IPC_PING_REQ};
        int len = send(message);
        if (len >= 0) {
            auto rsp = receive(pingTimeout, {IPC_PING_RSP});
            if (rsp != nullptr) {
                LINX_INFO("[%s] connected", getName().c_str());
                return true;
            }
        }
    } while (!deadline.isExpired());

    LINX_ERROR("[%s] connection timed out", getName().c_str());
    return false;
}

template<typename IdentifierType>
bool GenericClient<IdentifierType>::isEqual(const LinxClient &other) const {
    const auto *otherClient = dynamic_cast<const GenericClient<IdentifierType>*>(&other);
    return this->socket == otherClient->socket && this->identifier == otherClient->identifier;
}

template<typename IdentifierType>
std::string GenericClient<IdentifierType>::getName() const {
    return clientId;
}
