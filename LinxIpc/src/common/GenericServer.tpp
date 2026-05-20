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

template<typename IdentifierType>
void GenericServer<IdentifierType>::task() {

    LINX_INFO("[%s] Task started", this->getName().c_str());
    while (true) {
        RawMessagePtr msg{};
        std::unique_ptr<IIdentifier> from {};

        int ret = this->socket->receive(&msg, &from, INFINITE_TIMEOUT);
        if (ret == 0) {
            LINX_INFO("[%s] socket closed, Task stopping", this->getName().c_str());
            break;
        }
        if (ret < 0) {
            LINX_ERROR("[%s] receive error: %d, Task stopping", this->getName().c_str(), ret);
            break;
        }

        auto reqId = msg->getReqId();
        if (reqId == IPC_PING_REQ) {
            RawMessage rsp = RawMessage(IPC_PING_RSP);
            this->send(rsp, *from);
            continue;
        }

        auto container = std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
            .message = std::move(msg),
            .from = std::move(from),
            .server = this->weak_from_this()
        });
        if (queue->add(std::move(container)) != 0) {
            LINX_ERROR("[%s] Received reqId: 0x%x from: %s discarded - queue full",
                      this->getName().c_str(), reqId, container->from->format().c_str());
        }
    }
}

template<typename IdentifierType>
GenericServer<IdentifierType>::GenericServer(
    const std::string &serverId,
    const std::shared_ptr<GenericSocket<IdentifierType>> &socket,
    std::unique_ptr<LinxQueue> &&queue)
    : GenericSimpleServer<IdentifierType>(serverId, socket) {
    assert(queue);
    this->queue = std::move(queue);
}

template<typename IdentifierType>
GenericServer<IdentifierType>::~GenericServer() {
    stop();
}

template<typename IdentifierType>
bool GenericServer<IdentifierType>::start() {
    if (workerThread.joinable()) {
        return true;
    }

    LINX_INFO("[%s] Starting worker thread", this->getName().c_str());
    workerThread = std::thread([this]() { this->task(); });
    return true;
}

template<typename IdentifierType>
void GenericServer<IdentifierType>::stop() {
    if (workerThread.joinable()) {
        LINX_INFO("[%s] Stopping worker thread", this->getName().c_str());
        this->socket->close();
        this->queue->stop();
        workerThread.join();
    }
}

template<typename IdentifierType>
int GenericServer<IdentifierType>::getPollFd() const {
    return queue->getFd();
}

template<typename IdentifierType>
LinxReceivedMessageSharedPtr GenericServer<IdentifierType>::receive(
    int timeoutMs,
    const std::vector<uint32_t> &sigsel,
    const IIdentifier *from) {

    auto recvMsg = queue->get(timeoutMs, sigsel, from);
    if (recvMsg != nullptr) {
        LINX_DEBUG("[%s] Received reqId: 0x%x", this->getName().c_str(), recvMsg->message->getReqId());
        return recvMsg;
    }

    return recvMsg;
}
