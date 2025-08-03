#pragma once

#include <list>
#include <pthread.h>
#include "LinxQueue.h"
#include "LinxIpcEventFd.h"

class LinxQueueImpl : public LinxQueue {
   public:
    LinxQueueImpl(LinxIpcEventFd *efd, int size);
    ~LinxQueueImpl();
    int add(const std::shared_ptr<LinxMessageIpc> &msg) override;
    int size() const override;
    int getFd() const override;
    void clear() override;
    LinxMessageIpcPtr get(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                        const std::optional<LinxIpcClientPtr> &from) override;

   private:
    LinxIpcEventFd *efd;
    int max_size = 0;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cv;
    std::list<LinxMessageIpcPtr> queue;

    LinxMessageIpcPtr findMessage(const std::vector<uint32_t> &sigsel, const std::optional<LinxIpcClientPtr> &from);
    LinxMessageIpcPtr waitForMessage(int timeoutMs, const std::vector<uint32_t> &sigsel, const std::optional<LinxIpcClientPtr> &from);
    LinxMessageIpcPtr waitForMessage(const std::vector<uint32_t> &sigsel, const std::optional<LinxIpcClientPtr> &from);
    LinxMessageIpcPtr getMessage(const std::vector<uint32_t> &sigsel, const std::optional<LinxIpcClientPtr> &from);
};

LinxQueue* createIpcQueue(int maxSize);