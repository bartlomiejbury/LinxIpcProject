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
    int add(const std::shared_ptr<LinxMessageIpc> &msg, const std::string &clientName) override;
    int size() const override;
    int getFd() const override;
    void clear() override;
    LinxQueueElement get(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                        const std::optional<std::string> &from) override;

   private:
   LinxQueueFd *efd;
    int max_size = 0;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cv;
    std::list<LinxQueueElement> queue;

    LinxQueueElement findMessage(const std::vector<uint32_t> &sigsel, const std::optional<std::string> &from);
    LinxQueueElement waitForMessage(int timeoutMs, const std::vector<uint32_t> &sigsel, const std::optional<std::string> &from);
    LinxQueueElement waitForMessage(const std::vector<uint32_t> &sigsel, const std::optional<std::string> &from);
    LinxQueueElement getMessage(const std::vector<uint32_t> &sigsel, const std::optional<std::string> &from);
};
