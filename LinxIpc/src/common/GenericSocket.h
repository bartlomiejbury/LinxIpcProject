#pragma once

#include "LinxIpc.h"

template<typename IdentifierType>
class GenericSocket {
  public:
    using Identifier = IdentifierType;

    virtual ~GenericSocket() = default;
    virtual int open() = 0;
    virtual void close() = 0;

    virtual int getFd() const = 0;

    virtual int send(const IMessage &message, const Identifier &to) = 0;
    virtual int receive(RawMessagePtr *msg, Identifier *from, int timeout) = 0;

    virtual int flush() = 0;
};
