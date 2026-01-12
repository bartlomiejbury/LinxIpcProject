#pragma once

#include <cassert>
#include "LinxIpc.h"
#include "LinxMessageIds.h"
#include "LinxTrace.h"
#include "LinxMessageFilter.h"
#include "Deadline.h"

template<typename SocketType>
GenericSimpleServer<SocketType>::GenericSimpleServer(
    const std::string &serverId,
    const std::shared_ptr<SocketType> &socket) {
    assert(socket);
    this->serverId = serverId;
    this->socket = socket;
}

template<typename SocketType>
GenericSimpleServer<SocketType>::~GenericSimpleServer() {
    stop();
}

template<typename SocketType>
bool GenericSimpleServer<SocketType>::start() {
    // Direct mode server doesn't need to start anything
    return true;
}

template<typename SocketType>
void GenericSimpleServer<SocketType>::stop() {
    // Direct mode server doesn't need to stop anything
}

template<typename SocketType>
int GenericSimpleServer<SocketType>::getPollFd() const {
    return socket->getFd();
}

template<typename SocketType>
int GenericSimpleServer<SocketType>::send(const IMessage &message, const IIdentifier &to) {

    // Downcast to the concrete identifier type
    const auto *typedTo = dynamic_cast<const IdentifierType*>(&to);
    if (typedTo) {
        LINX_DEBUG("[%s] Sending message to: %s, reqId: 0x%x",
                  getName().c_str(), typedTo->format().c_str(), message.getReqId());
        auto ret = socket->send(message, *typedTo);
        if (ret < 0) {
            LINX_ERROR("[%s] send error: %d", getName().c_str(), ret);
        }
        return ret;
    }

    LINX_ERROR("[%s] send failed - invalid identifier type", getName().c_str());
    return -1;
}

template<typename SocketType>
LinxReceivedMessageSharedPtr GenericSimpleServer<SocketType>::receive(
    int timeoutMs,
    const std::vector<uint32_t> &sigsel,
    const IIdentifier *identifier) {

    RawMessagePtr msg{};
    std::unique_ptr<IIdentifier> from;

    auto predicate = [identifier, &sigsel](const RawMessagePtr &msg, const std::unique_ptr<IIdentifier> &from) {
        return LinxMessageFilter::matchesFrom(from.get(), identifier) &&
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

        auto reqId = msg->getReqId();
        if (reqId == IPC_PING_REQ) {
            RawMessage rsp = RawMessage(IPC_PING_RSP);
            send(rsp, *from);
            continue;
        }

        if (predicate(msg, from)) {
            return std::make_shared<LinxReceivedMessage>(LinxReceivedMessage{
                .message = std::move(msg),
                .from = std::move(from),
                .server = this->weak_from_this()
            });
        }
    } while (!deadline.isExpired());

    LINX_ERROR("[%s] receive timed out", getName().c_str());
    return nullptr;
}

template<typename SocketType>
std::string GenericSimpleServer<SocketType>::getName() const {
    return serverId;
}
