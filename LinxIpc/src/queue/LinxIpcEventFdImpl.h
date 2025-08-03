#pragma once

#include "LinxIpcEventFd.h"

class LinxIpcEventFdImpl : public LinxIpcEventFd {
  public:
    LinxIpcEventFdImpl();
    ~LinxIpcEventFdImpl();

    int getFd() const override;
    int writeEvent() override;
    int readEvent() override;
    void clearEvents() override;

  private:
    int efd;
};
