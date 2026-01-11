#pragma once

#include <cassert>
#include "LinxEventFd.h"
#include "LinxIpc.h"
#include "LinxMessageIds.h"
#include "LinxQueue.h"
#include "LinxTrace.h"
#include <stdio.h>

template<typename SocketType>
void GenericServer<SocketType>::task() {

    LINX_INFO("[%s] Task started", getName().c_str());
    while (true) {
        RawMessagePtr msg{};
        std::unique_ptr<IIdentifier> from {};

        int ret = socket->receive(&msg, &from, INFINITE_TIMEOUT);
        if (ret == 0) {
            LINX_INFO("[%s] socket closed, Task stopping", getName().c_str());
            break;
        }
        if (ret < 0) {
            LINX_ERROR("[%s] receive error: %d, Task stopping", getName().c_str(), ret);
            break;
        }

        auto reqId = msg->getReqId();
        if (reqId == IPC_PING_REQ) {
            RawMessage rsp = RawMessage(IPC_PING_RSP);
            send(rsp, *from);
            continue;
        }

        auto container = std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
            .message = std::move(msg),
            .from = std::move(from),
            .server = this->weak_from_this()
        });
        if (queue->add(std::move(container)) != 0) {
            LINX_ERROR("[%s] Received reqId: 0x%x from: %s discarded - queue full",
                      getName().c_str(), reqId, container->from->format().c_str());
        }
    }
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
    return queue->getFd();
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
LinxReceivedMessageSharedPtr GenericServer<SocketType>::receive(
    int timeoutMs,
    const std::vector<uint32_t> &sigsel,
    const IIdentifier *from) {

    auto recvMsg = queue->get(timeoutMs, sigsel, from);
    if (recvMsg != nullptr) {
        LINX_DEBUG("[%s] Received reqId: 0x%x", getName().c_str(), recvMsg->message->getReqId());
        return recvMsg;
    }

    return recvMsg;
}


template<typename SocketType>
std::string GenericServer<SocketType>::getName() const {
    return serverId;
}