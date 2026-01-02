#include <cassert>
#include "LinxEventFd.h"
#include "LinxIpc.h"
#include "LinxQueue.h"
#include "IIdentifier.h"

LinxQueue::LinxQueue(std::unique_ptr<LinxEventFd> &&efd, int size): efd{std::move(efd)}, max_size{size} {
    assert(this->efd);
}

LinxQueue::~LinxQueue() {
    stop();
}

int LinxQueue::add(LinxReceivedMessagePtr &&msg) {
    assert(msg);

    std::unique_lock<std::mutex> lock(m_mutex);

    int result = -1;
    if (queue.size() < (std::size_t)max_size) {
        queue.push_back(std::move(msg));
        efd->writeEvent();
        result = 0;
    }

    lock.unlock();
    m_cv.notify_all();
    return result;
};

void LinxQueue::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    stopped = true;
    queue.clear();
    efd->clearEvents();
    m_cv.notify_all();
}

void LinxQueue::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    queue.clear();
    efd->clearEvents();
}

LinxReceivedMessagePtr LinxQueue::get(int timeoutMs, const std::vector<uint32_t> &sigsel, const IIdentifier *from) {
    if (timeoutMs == IMMEDIATE_TIMEOUT) {
        return getMessage(sigsel, from);
    } else if (timeoutMs == INFINITE_TIMEOUT) {
        return waitForMessage(sigsel, from);
    } else {
        return waitForMessage(timeoutMs, sigsel, from);
    }
}

LinxReceivedMessagePtr LinxQueue::getMessage(const std::vector<uint32_t> &sigsel,
                                           const IIdentifier *from) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return findMessage(sigsel, from);
}

LinxReceivedMessagePtr LinxQueue::waitForMessage(const std::vector<uint32_t> &sigsel, const IIdentifier *from) {

    LinxReceivedMessagePtr msg = nullptr;
    std::unique_lock<std::mutex> lock(m_mutex);

    m_cv.wait(lock, [this, &sigsel, &from, &msg]() {
        msg = findMessage(sigsel, from);
        return msg != nullptr || stopped;
    });

    return msg;
};

LinxReceivedMessagePtr LinxQueue::waitForMessage(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                                               const IIdentifier *from) {

    LinxReceivedMessagePtr msg = nullptr;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto timeout = std::chrono::milliseconds(timeoutMs);

    m_cv.wait_for(lock, timeout, [this, &sigsel, &from, &msg]() {
        msg = findMessage(sigsel, from);
        return msg != nullptr || stopped;
    });

    return msg;
};

LinxReceivedMessagePtr LinxQueue::findMessage(const std::vector<uint32_t> &sigsel, const IIdentifier *from) {

    auto predicate = [&sigsel, &from](LinxReceivedMessagePtr &msg) {
        if (from != nullptr) {
            if (!(*msg->from == *from)) {
                return false;
            }
        }

        uint32_t reqId = msg->message->getReqId();
        return sigsel.size() == 0 || std::find_if(sigsel.begin(), sigsel.end(),
                                                      [reqId](uint32_t id) { return id == reqId; }) != sigsel.end();
    };

    if (auto it = std::find_if(queue.begin(), queue.end(), predicate); it != queue.end()) {
        auto msg = std::move(*it);
        queue.erase(it);
        efd->readEvent();
        return msg;
    }

    return nullptr;
}

int LinxQueue::size() const {
    return queue.size();
}

int LinxQueue::getFd() const {
    return efd->getFd();
}
