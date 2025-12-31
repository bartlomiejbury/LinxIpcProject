#pragma once

#include "gmock/gmock.h"
#include "UdpSocket.h"

class UdpSocketMock : public UdpSocket {
  public:
    MOCK_METHOD(int, open, (), (override));
    MOCK_METHOD(int, bind, (uint16_t port), (override));
    MOCK_METHOD(int, joinMulticastGroup, (const std::string &multicastAddress), (override));
    MOCK_METHOD(int, setMulticastTtl, (int ttl), (override));
    MOCK_METHOD(int, setBroadcast, (bool enable), (override));
    MOCK_METHOD(int, getFd, (), (const, override));
    MOCK_METHOD(int, receive, (RawMessagePtr *msg, PortInfo *from, int timeoutMs), (override));
    MOCK_METHOD(int, send, (const IMessage &message, const PortInfo &to), (override));
};
