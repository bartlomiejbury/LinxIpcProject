#pragma once

#include <list>
#include <pthread.h>
#include "LinxQueue.h"
#include "LinxQueueFd.h"
#include "LinxTime.h"

class LinxQueueImpl : public LinxQueue {
   public:
    LinxQueueImpl(LinxQueueFd *efd, int size);
    ~LinxQueueImpl();
    int add(const std::shared_ptr<LinxMessageIpc> &msg) override;
    int size() const override;
    int getFd() const override;
    void clear() override;
    LinxMessageIpcPtr get(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                            const LinxIpcClientPtr &from) override;

   private:
   LinxQueueFd *efd;
    int max_size = 0;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cv;
    std::list<LinxMessageIpcPtr> queue;

    LinxMessageIpcPtr findMessage(const std::vector<uint32_t> &sigsel,
                                                    const LinxIpcClientPtr &from);
    LinxMessageIpcPtr waitForMessage(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                                       const LinxIpcClientPtr &from);
    LinxMessageIpcPtr waitForMessage(const std::vector<uint32_t> &sigsel,
                                                       const LinxIpcClientPtr &from);
};
