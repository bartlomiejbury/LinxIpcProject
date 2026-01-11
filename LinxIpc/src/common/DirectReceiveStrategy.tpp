#pragma once

#include "DirectReceiveStrategy.h"
#include "LinxTrace.h"
#include "LinxMessageFilter.h"
#include "LinxMessageIds.h"
#include "RawMessage.h"
#include "Deadline.h"
#include "IIdentifier.h"

template<typename SocketType>
DirectReceiveStrategy<SocketType>::DirectReceiveStrategy(
    const std::shared_ptr<SocketType> &socket)
    : socket(socket) {
}

template<typename SocketType>
LinxContainerPtr DirectReceiveStrategy<SocketType>::receive(
    int timeoutMs,
    const std::vector<uint32_t> &sigsel,
    const IIdentifier *from) {

    RawMessagePtr msg{};
    std::unique_ptr<IIdentifier> fromIdentifier;

    auto predicate = [from, &sigsel](const RawMessagePtr &msg, const std::unique_ptr<IIdentifier> &fromId) {
        return LinxMessageFilter::matchesFrom(fromId.get(), from) &&
               LinxMessageFilter::matchesSignalSelector(*msg, sigsel);
    };

    auto deadline = Deadline(timeoutMs);
    do {
        int timeout = deadline.getRemainingTimeMs();
        int ret = socket->receive(&msg, &fromIdentifier, timeout);
        if (ret == 0) {
            LINX_DEBUG("receive timeout");
            return nullptr;
        }
        if (ret < 0) {
            LINX_ERROR("receive error: %d", ret);
            return nullptr;
        }

        auto reqId = msg->getReqId();
        if (reqId == IPC_PING_REQ) {
            RawMessage rsp = RawMessage(IPC_PING_RSP);
            using IdentifierType = typename SocketTraits<SocketType>::Identifier;
            auto *typedFrom = dynamic_cast<IdentifierType*>(fromIdentifier.get());
            if (typedFrom) {
                socket->send(rsp, *typedFrom);
            }
            continue;
        }

        if (predicate(msg, fromIdentifier)) {
            return std::make_shared<LinxContainer>(LinxContainer{
                .message = std::move(msg),
                .from = std::move(fromIdentifier),
            });
        }
    } while (!deadline.isExpired());

    LINX_ERROR("receive timed out");
    return nullptr;
}

template<typename SocketType>
int DirectReceiveStrategy<SocketType>::getPollFd() const {
    return socket->getFd();
}

template<typename SocketType>
bool DirectReceiveStrategy<SocketType>::start() {
    // Direct strategy has no background thread, nothing to start
    return true;
}

template<typename SocketType>
void DirectReceiveStrategy<SocketType>::stop() {
    // Direct strategy has no background thread, nothing to stop
}
