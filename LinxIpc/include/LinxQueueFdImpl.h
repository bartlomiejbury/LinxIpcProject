#pragma once

#include "LinxQueueFd.h"

class LinxQueueFdImpl : public LinxQueueFd {
  public:
    LinxQueueFdImpl();
    ~LinxQueueFdImpl();

    int getFd() const override;
    int writeEvent() override;
    int readEvent() override;
    void clearEvents() override;

  private:
    int efd;
};
