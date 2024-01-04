#pragma once

class LinxQueueFd {
  public:
    LinxQueueFd();
    ~LinxQueueFd();

    int getFd() const;
    void writeEvent();
    void readEvent();
    void clearEvents();

  private:
    int efd;
};
