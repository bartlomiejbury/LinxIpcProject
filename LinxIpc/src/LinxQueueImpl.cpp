#include <time.h>
#include "LinxQueueImpl.h"

LinxQueueImpl::LinxQueueImpl(int size) {
    max_size = size;
    pthread_mutex_init(&m_mutex, NULL);

    pthread_condattr_t attr{};
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_cv, &attr);
}

int LinxQueueImpl::add(LinxMessageIpc *msg) {
    pthread_mutex_lock(&m_mutex);

    int result = -1;
    if (queue.size() < (std::size_t)max_size) {
        queue.push_back(msg);
        result = 0;
    }

    pthread_mutex_unlock(&m_mutex);
    pthread_cond_broadcast(&m_cv);
    return result;
};

LinxMessageIpc *LinxQueueImpl::get(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) {
    if (timeoutMs == INFINITE_TIMEOUT) {
        return waitForMessage(sigsel, from);
    } else {
        return waitForMessage(timeoutMs, sigsel, from);
    }
}

LinxMessageIpc* LinxQueueImpl::waitForMessage(const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) {
    pthread_mutex_lock(&m_mutex);

    LinxMessageIpc *msg = nullptr;
    while ((msg = findMessage(sigsel, from)) == nullptr) {
        pthread_cond_wait(&m_cv, &m_mutex);
    }
    
    pthread_mutex_unlock(&m_mutex);
    return msg;
};

LinxMessageIpc* LinxQueueImpl::waitForMessage(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) {
    pthread_mutex_lock(&m_mutex);

    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);

    uint64_t seconds = timeoutMs / MILLI_SECONDS;
    uint64_t nano_seconds = (timeoutMs % MILLI_SECONDS) * (NANO_SECONDS / MILLI_SECONDS);

    if (ts.tv_nsec + nano_seconds > NANO_SECONDS) {
        seconds = seconds + 1;
        nano_seconds = ts.tv_nsec + nano_seconds - NANO_SECONDS;
    }

    ts.tv_sec += seconds;
    ts.tv_nsec += nano_seconds;

    LinxMessageIpc *msg = nullptr;
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

LinxMessageIpc* LinxQueueImpl::findMessage(const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) {

    auto it = std::find_if(queue.begin(), queue.end(), [sigsel, from](LinxMessageIpc *msg) {
        if (from == nullptr || msg->getClient()->getName() == from->getName()) {

            if (sigsel.size() == 0) {
                return true;
            }

            uint32_t reqId = msg->getReqId();
            for (uint32_t id : sigsel) {
                if (reqId == id) {
                    return true;
                }
            }
        }

        return false;
    });

    if (it != queue.end()) {
        queue.erase(it);
        return *it;
    }

    return nullptr;
}
