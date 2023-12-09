#pragma once

#include <list>
#include <pthread.h>
#include <initializer_list>
#include "LinxQueue.h"

class LinxQueueImpl: public LinxQueue {
  public:
    LinxQueueImpl(int size);
    int add(LinxMessageIpc *msg) override;
    int size() override;
    LinxMessageIpc *get(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from) override;

  private:
    int max_size;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cv;
    std::list<LinxMessageIpc *> queue;

    LinxMessageIpc* findMessage(const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from);
    LinxMessageIpc* waitForMessage(int timeoutMs, const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from);
    LinxMessageIpc* waitForMessage(const std::initializer_list<uint32_t> &sigsel, LinxIpcClient *from);
};
