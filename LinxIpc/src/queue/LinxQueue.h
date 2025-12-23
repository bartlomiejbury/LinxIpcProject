#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include "LinxIpc.h"

class LinxEventFd;

using LinxReceiveContextOpt = std::optional<LinxReceiveContextSharedPtr>;

class LinxQueue {
   public:
    LinxQueue(std::unique_ptr<LinxEventFd> &&efd, int size);
    virtual ~LinxQueue();
    virtual int add(LinxReceivedMessagePtr &&msg);
    virtual int size() const;
    virtual int getFd() const;
    virtual void clear();
    virtual LinxReceivedMessagePtr get(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                   const LinxReceiveContextOpt &from);

   private:
    std::unique_ptr<LinxEventFd> efd;
    int max_size = 0;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::list<LinxReceivedMessagePtr> queue;

    LinxReceivedMessagePtr findMessage(const std::vector<uint32_t> &sigsel, const LinxReceiveContextOpt &from);
    LinxReceivedMessagePtr waitForMessage(int timeoutMs, const std::vector<uint32_t> &sigsel, const LinxReceiveContextOpt &from);
    LinxReceivedMessagePtr waitForMessage(const std::vector<uint32_t> &sigsel, const LinxReceiveContextOpt &from);
    LinxReceivedMessagePtr getMessage(const std::vector<uint32_t> &sigsel, const LinxReceiveContextOpt &from);
};