#pragma once

#include "QueueReceiveStrategy.h"
#include "LinxQueue.h"
#include "LinxTrace.h"
#include "IIdentifier.h"
#include "LinxMessageFilter.h"
#include "LinxMessageIds.h"
#include "RawMessage.h"
#include "Deadline.h"

template<typename SocketType>
QueueReceiveStrategy<SocketType>::QueueReceiveStrategy(
    std::unique_ptr<LinxQueue> &&queue,
    const std::shared_ptr<SocketType> &socket)
    : queue(std::move(queue)),
      socket(socket) {
}

template<typename SocketType>
QueueReceiveStrategy<SocketType>::~QueueReceiveStrategy() {
    stop();
}

template<typename SocketType>
LinxContainerPtr QueueReceiveStrategy<SocketType>::receive(
    int timeoutMs,
    const std::vector<uint32_t> &sigsel,
    const IIdentifier *from) {

    auto recvMsg = queue->get(timeoutMs, sigsel, from);
    if (recvMsg != nullptr) {
        LINX_DEBUG("Received reqId: 0x%x from queue", recvMsg->message->getReqId());
        return recvMsg;
    }

    return recvMsg;
}

template<typename SocketType>
int QueueReceiveStrategy<SocketType>::getPollFd() const {
    return queue->getFd();
}

template<typename SocketType>
void QueueReceiveStrategy<SocketType>::task() {
    LINX_INFO("Task started");
    while (true) {
        RawMessagePtr msg{};
        std::unique_ptr<IIdentifier> from {};

        int ret = socket->receive(&msg, &from, INFINITE_TIMEOUT);
        if (ret == 0) {
            LINX_INFO("socket closed, Task stopping");
            break;
        }
        if (ret < 0) {
            LINX_ERROR("receive error: %d, Task stopping", ret);
            break;
        }

        auto reqId = msg->getReqId();
        if (reqId == IPC_PING_REQ) {
            RawMessage rsp = RawMessage(IPC_PING_RSP);
            using IdentifierType = typename SocketTraits<SocketType>::Identifier;
            auto *typedFrom = dynamic_cast<IdentifierType*>(from.get());
            if (typedFrom) {
                socket->send(rsp, *typedFrom);
            }
        }

        auto container = std::make_unique<LinxContainer>(LinxContainer{
            .message = std::move(msg),
            .from = std::move(from)
        });
        if (queue->add(std::move(container)) != 0) {
            LINX_ERROR("Received reqId: 0x%x from: %s discarded - queue full",
                      reqId, container->from->format().c_str());
        }
    }
}

template<typename SocketType>
bool QueueReceiveStrategy<SocketType>::start() {
    if (workerThread.joinable()) {
        return true;
    }

    LINX_INFO("Starting worker thread");
    workerThread = std::thread([this]() { this->task(); });
    return true;
}

template<typename SocketType>
void QueueReceiveStrategy<SocketType>::stop() {
    if (workerThread.joinable()) {
        LINX_INFO("Stopping worker thread");
        socket->close();
        queue->stop();
        workerThread.join();
    }
}
