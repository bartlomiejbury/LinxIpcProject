#pragma once

#include <cstring>
#include "Deadline.h"
#include "LinxTrace.h"
#include "LinxMessageIds.h"
#include "GenericSocket.h"

template<typename SocketType>
GenericClient<SocketType>::GenericClient(const std::string &clientId,
                                         const std::shared_ptr<SocketType> &socket,
                                         const IdentifierType &identifier)
    : clientId(clientId), socket(socket), identifier(identifier) {
}

template<typename SocketType>
GenericClient<SocketType>::~GenericClient() {
    LINX_INFO("[%s] Stopping", getName().c_str());
    this->socket->close();
}

template<typename SocketType>
int GenericClient<SocketType>::send(const IMessage &message) {
    LINX_DEBUG("[%s] Sending message reqId: 0x%x",
            getName().c_str(), message.getReqId());
    auto ret = socket->send(message, identifier);
    if (ret < 0) {
        LINX_ERROR("[%s] Send error: %d", getName().c_str(), ret);
    }
    return ret;
}

template<typename SocketType>
RawMessagePtr GenericClient<SocketType>::receive(int timeoutMs, const std::vector<uint32_t> &sigsel) {
    RawMessagePtr msg{};
    std::unique_ptr<IIdentifier> from;

    auto predicate = [this, sigsel](RawMessagePtr &msg, const std::vector<uint32_t> &sigselArg, const std::unique_ptr<IIdentifier> &from) {
        if (from && *from == this->identifier) {
            uint32_t reqId = msg->getReqId();
            return sigselArg.size() == 0 || std::find_if(sigselArg.begin(), sigselArg.end(),
                    [reqId](uint32_t id) { return id == reqId; }) != sigselArg.end();
        }
        return false;
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
        if (predicate(msg, sigsel, from)) {
            return msg;
        }
    } while (!deadline.isExpired());

    LINX_ERROR("[%s] receive timed out", getName().c_str());
    return nullptr;
}

template<typename SocketType>
RawMessagePtr GenericClient<SocketType>::sendReceive(const IMessage &message, int timeoutMs,
                                                                        const std::vector<uint32_t> &sigsel) {
    if (send(message) < 0) {
        return nullptr;
    }
    return receive(timeoutMs, sigsel);
}

template<typename SocketType>
bool GenericClient<SocketType>::connect(int timeoutMs) {
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

template<typename SocketType>
bool GenericClient<SocketType>::isEqual(const LinxClient &other) const {
    const auto *otherClient = dynamic_cast<const GenericClient<SocketType>*>(&other);
    return this->socket == otherClient->socket && this->identifier == otherClient->identifier;
}

template<typename SocketType>
std::string GenericClient<SocketType>::getName() const {
    return clientId;
}
