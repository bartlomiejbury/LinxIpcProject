#pragma once

#include "LinxIpc.h"

class LinxIpcSocket {
  public:
    virtual ~LinxIpcSocket() {};

    virtual std::string getName() = 0;
    virtual int getFd() = 0;

    virtual int send(const LinxMessageIpc *message, const std::string &to) = 0;
    virtual int receive(LinxMessageIpc **msg, std::string *from, int timeout) = 0;

    virtual int flush() = 0;
};
