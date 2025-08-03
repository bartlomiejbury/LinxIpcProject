#pragma once

class LinxIpcEventFd {
  public:
    virtual ~LinxIpcEventFd() {};

    virtual int getFd() const = 0;
    virtual int writeEvent() = 0;
    virtual int readEvent() = 0;
    virtual void clearEvents() = 0;
};
