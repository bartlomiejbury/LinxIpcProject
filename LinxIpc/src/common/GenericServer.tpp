#pragma once

#include <cassert>
#include "LinxEventFd.h"
#include "LinxIpc.h"
#include "LinxMessageIds.h"
#include "LinxQueue.h"
#include "LinxTrace.h"
#include "LinxMessageFilter.h"
#include "Deadline.h"
#include <stdio.h>

template<typename SocketType>
void GenericServer<SocketType>::task() {

    LINX_INFO("[%s] Task started", getName().c_str());
    while (true) {
        auto container = receiveFromSocket(INFINITE_TIMEOUT, LINX_ANY_SIG, LINX_ANY_FROM);
        if (!container) {
            break;
        }

        uint32_t reqId = container->message->getReqId();
        (void)reqId;  // Suppress unused variable warning
        if (queue->add(std::move(container)) != 0) {
            LINX_ERROR("[%s] Received reqId: 0x%x discarded - queue full",
                      getName().c_str(), reqId);
        }
    }
    LINX_INFO("[%s] Task stopped", getName().c_str());
}

template<typename SocketType>
GenericServer<SocketType>::GenericServer(
    const std::string &serverId,
    const std::shared_ptr<SocketType> &socket,
    std::unique_ptr<LinxQueue> &&queue) {
    assert(queue);
    assert(socket);
    this->serverId = serverId;
    this->socket = std::move(socket);
    this->queue = std::move(queue);
}

template<typename SocketType>
GenericServer<SocketType>::~GenericServer() {
    stop();
}

template<typename SocketType>
bool GenericServer<SocketType>::start() {
    if (workerThread.joinable()) {
        return true;
    }

    LINX_INFO("[%s] Starting", getName().c_str());
    workerThread = std::thread([this]() { this->task(); });
    return true;
}

template<typename SocketType>
void GenericServer<SocketType>::stop() {
    if (workerThread.joinable()) {
        LINX_INFO("[%s] Stopping", getName().c_str());
        this->socket->close();
        this->queue->stop();
        workerThread.join();
    }
}

template<typename SocketType>
int GenericServer<SocketType>::getPollFd() const {
    if (workerThread.joinable()) {
        return queue->getFd();
    }
    return socket->getFd();
}

template<typename SocketType>
int GenericServer<SocketType>::send(const IMessage &message, const IIdentifier &to) {

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
LinxReceivedMessageSharedPtr GenericServer<SocketType>::receiveFromTask(
    int timeoutMs,
    const std::vector<uint32_t> &sigsel,
    const IIdentifier *identifier) {

    auto recvMsg = queue->get(timeoutMs, sigsel, identifier);
    if (recvMsg != nullptr) {
        LINX_DEBUG("[%s] Received reqId: 0x%x", getName().c_str(), recvMsg->message->getReqId());
        return recvMsg;
    }

    return recvMsg;
}

template<typename SocketType>
LinxReceivedMessagePtr GenericServer<SocketType>::receiveFromSocket(
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
            return std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
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
LinxReceivedMessageSharedPtr GenericServer<SocketType>::receive(
    int timeoutMs,
    const std::vector<uint32_t> &sigsel,
    const IIdentifier *from) {

    if (workerThread.joinable()) {
        return receiveFromTask(timeoutMs, sigsel, from);
    } else {
        return receiveFromSocket(timeoutMs, sigsel, from);
    }

    LINX_ERROR("[%s] receive failed - invalid identifier type", getName().c_str());
    return nullptr;
}

template<typename SocketType>
std::string GenericServer<SocketType>::getName() const {
    return serverId;
}