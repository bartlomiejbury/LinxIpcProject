#include "LinxQueueImpl.h"

LinxQueueImpl::LinxQueueImpl(int size) {
    max_size = size;
    pthread_mutex_init(&m_mutex, NULL);

    pthread_condattr_t attr{};
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_cv, &attr);
}

LinxQueueImpl::~LinxQueueImpl() {
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cv);
}

int LinxQueueImpl::add(const LinxMessageIpcPtr &msg, const std::string &from) {
    pthread_mutex_lock(&m_mutex);

    int result = -1;
    if (queue.size() < (std::size_t)max_size) {
        queue.push_back(std::make_shared<LinxQueueContainer>(msg, from));
        result = 0;
    }

    pthread_mutex_unlock(&m_mutex);
    pthread_cond_broadcast(&m_cv);
    return result;
};

void LinxQueueImpl::clear() {
    pthread_mutex_lock(&m_mutex);
    queue.clear();
    pthread_mutex_unlock(&m_mutex);
}

std::shared_ptr<LinxQueueContainer> LinxQueueImpl::get(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, const std::optional<std::string> &from) {
    if (timeoutMs == IMMEDIATE_TIMEOUT) {
        return findMessage(sigsel, from);
    } else if (timeoutMs == INFINITE_TIMEOUT) {
        return waitForMessage(sigsel, from);
    } else {
        return waitForMessage(timeoutMs, sigsel, from);
    }
}

std::shared_ptr<LinxQueueContainer> LinxQueueImpl::waitForMessage(const std::initializer_list<uint32_t> &sigsel, const std::optional<std::string> &from) {
    pthread_mutex_lock(&m_mutex);

    std::shared_ptr<LinxQueueContainer> msg = nullptr;
    while ((msg = findMessage(sigsel, from)) == nullptr) {
        pthread_cond_wait(&m_cv, &m_mutex);
    }
    
    pthread_mutex_unlock(&m_mutex);
    return msg;
};

std::shared_ptr<LinxQueueContainer> LinxQueueImpl::waitForMessage(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, const std::optional<std::string> &from) {
    pthread_mutex_lock(&m_mutex);

    uint64_t currentTime = getTimeMs();
    timespec ts = timeMsToTimespec(currentTime + timeoutMs);

    std::shared_ptr<LinxQueueContainer> msg = nullptr;
    while ((msg = findMessage(sigsel, from)) == nullptr) {
        int ret = pthread_cond_timedwait(&m_cv, &m_mutex, &ts);
        if (ret == ETIMEDOUT) {
            break;
        }
    }
    
    pthread_mutex_unlock(&m_mutex);
    return msg;
};

int LinxQueueImpl::size() {
    return queue.size();
}

std::shared_ptr<LinxQueueContainer> LinxQueueImpl::findMessage(const std::initializer_list<uint32_t> &sigsel, const std::optional<std::string> &from) {

    auto it = std::find_if(queue.begin(), queue.end(), [sigsel, from](std::shared_ptr<LinxQueueContainer> &msg) {
        if (!from.has_value() || msg->from == from.value()) {

            if (sigsel.size() == 0) {
                return true;
            }

            uint32_t reqId = msg->message->getReqId();
            for (uint32_t id : sigsel) {
                if (reqId == id) {
                    return true;
                }
            }
        }

        return false;
    });

    if (it != queue.end()) {
        std::shared_ptr<LinxQueueContainer> container = *it;
        queue.erase(it);
        return container;
    }

    return nullptr;
}
