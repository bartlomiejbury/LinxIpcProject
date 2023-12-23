#pragma once

#include <list>
#include <pthread.h>
#include <initializer_list>
#include "LinxQueue.h"
#include "LinxTime.h"

class LinxQueueImpl: public LinxQueue {
  public:
    LinxQueueImpl(int size);
    ~LinxQueueImpl();
    int add(const std::shared_ptr<LinxMessageIpc> &msg, const std::string &from) override;
    int size() override;
    void clear() override;
    std::shared_ptr<LinxQueueContainer> get(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) override;

  private:

    int max_size;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cv;
    std::list<std::shared_ptr<LinxQueueContainer>> queue;

    std::shared_ptr<LinxQueueContainer> findMessage(const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from);
    std::shared_ptr<LinxQueueContainer> waitForMessage(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from);
    std::shared_ptr<LinxQueueContainer> waitForMessage(const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from);
};
