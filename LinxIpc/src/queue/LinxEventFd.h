#pragma once

class LinxEventFd {
  public:
    LinxEventFd();
    virtual ~LinxEventFd();

    virtual int getFd() const;
    virtual int writeEvent();
    virtual int readEvent();
    virtual void clearEvents();

  private:
    int efd;
};
