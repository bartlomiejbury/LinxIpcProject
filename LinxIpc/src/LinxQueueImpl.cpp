#include <cassert>
#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "trace.h"

LinxQueueImpl::LinxQueueImpl(LinxQueueFd *efd, int size): efd{efd}, max_size{size} {
    assert(efd);
    pthread_mutex_init(&m_mutex, NULL);

    pthread_condattr_t attr{};
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_cv, &attr);
}

LinxQueueImpl::~LinxQueueImpl() {
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cv);
    delete efd;
}

int LinxQueueImpl::add(const LinxMessageIpcPtr &msg) {
    assert(msg);

    pthread_mutex_lock(&m_mutex);

    int result = -1;
    if (queue.size() < (std::size_t)max_size) {
        queue.push_back(msg);
        efd->writeEvent();
        result = 0;
    }

    pthread_mutex_unlock(&m_mutex);
    pthread_cond_broadcast(&m_cv);
    return result;
};

void LinxQueueImpl::clear() {
    pthread_mutex_lock(&m_mutex);

    queue.clear();
    efd->clearEvents();

    pthread_mutex_unlock(&m_mutex);
}

LinxMessageIpcPtr LinxQueueImpl::get(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                                       const LinxIpcClientPtr &from) {
    if (timeoutMs == IMMEDIATE_TIMEOUT) {
        pthread_mutex_lock(&m_mutex);
        LinxMessageIpcPtr msg = findMessage(sigsel, from);
        pthread_mutex_unlock(&m_mutex);
        return msg;
    } else if (timeoutMs == INFINITE_TIMEOUT) {
        return waitForMessage(sigsel, from);
    } else {
        return waitForMessage(timeoutMs, sigsel, from);
    }
}

LinxMessageIpcPtr LinxQueueImpl::waitForMessage(const std::vector<uint32_t> &sigsel,
                                                                  const LinxIpcClientPtr &from) {
    pthread_mutex_lock(&m_mutex);

    LinxMessageIpcPtr msg = nullptr;
    while ((msg = findMessage(sigsel, from)) == nullptr) {
        pthread_cond_wait(&m_cv, &m_mutex);
    }

    pthread_mutex_unlock(&m_mutex);
    return msg;
};

LinxMessageIpcPtr LinxQueueImpl::waitForMessage(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                                               const LinxIpcClientPtr &from) {
    pthread_mutex_lock(&m_mutex);

    uint64_t currentTime = getTimeMs();
    timespec ts = timeMsToTimespec(currentTime + timeoutMs);

    LinxMessageIpcPtr msg = nullptr;
    while ((msg = findMessage(sigsel, from)) == nullptr) {
        int ret = pthread_cond_timedwait(&m_cv, &m_mutex, &ts);
        if (ret == ETIMEDOUT) {
            break;
        }
    }

    pthread_mutex_unlock(&m_mutex);
    return msg;
};

int LinxQueueImpl::size() const {
    return queue.size();
}

LinxMessageIpcPtr LinxQueueImpl::findMessage(const std::vector<uint32_t> &sigsel,
                                                               const LinxIpcClientPtr &from) {

    auto predicate = [&sigsel, &from](LinxMessageIpcPtr &msg) {
        if (!from || msg->getClient()->getName() == from->getName()) {
            uint32_t reqId = msg->getReqId();
            return sigsel.size() == 0 || std::find_if(sigsel.begin(), sigsel.end(),
                                                      [reqId](uint32_t id) { return id == reqId; }) != sigsel.end();
        }

        return false;
    };

    if (auto it = std::find_if(queue.begin(), queue.end(), predicate); it != queue.end()) {
        LinxMessageIpcPtr container = *it;
        queue.erase(it);
        efd->readEvent();
        return container;
    }

    return nullptr;
}

int LinxQueueImpl::getFd() const {
    return efd->getFd();
}
