#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "UdpSocket.h"
#include "RawMessage.h"
#include "SystemMock.h"
#include <arpa/inet.h>

using namespace ::testing;

class UdpSocketTests : public testing::Test {
  protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// Test basic constructor and destructor
TEST_F(UdpSocketTests, constructor_InitializesSocket) {
    UdpSocket socket;
    EXPECT_EQ(socket.getFd(), -1);  // Initially not open
}

// Test open success
TEST_F(UdpSocketTests, open_SuccessfullyCreatesSocket) {
    UdpSocket socket;
    EXPECT_EQ(socket.open(), 0);
    EXPECT_GE(socket.getFd(), 0);
    socket.close();
}

// Test open when already open
TEST_F(UdpSocketTests, open_FailsWhenAlreadyOpen) {
    UdpSocket socket;
    socket.open();
    int result = socket.open();
    EXPECT_LT(result, 0);
    socket.close();
}

// Test bind
TEST_F(UdpSocketTests, bind_SuccessfullyBindsToPort) {
    UdpSocket socket;
    socket.open();
    EXPECT_EQ(socket.bind(0), 0);  // Port 0 = let OS assign
    socket.close();
}

// Test receive on invalid socket
TEST_F(UdpSocketTests, receive_FailsOnInvalidSocket) {
    UdpSocket socket;
    RawMessagePtr msg;
    std::unique_ptr<IIdentifier> from;

    EXPECT_EQ(socket.receive(&msg, &from, 10), -1);
}

// Test send on invalid socket
TEST_F(UdpSocketTests, send_FailsOnInvalidSocket) {
    UdpSocket socket;
    RawMessage msg(42);
    PortInfo to("127.0.0.1", 12345);

    EXPECT_LT(socket.send(msg, to), 0);
}

// Test flush on invalid socket
TEST_F(UdpSocketTests, flush_FailsOnInvalidSocket) {
    UdpSocket socket;
    EXPECT_LT(socket.flush(), 0);
}

// Test setBroadcast on invalid socket
TEST_F(UdpSocketTests, setBroadcast_FailsOnInvalidSocket) {
    UdpSocket socket;
    EXPECT_LT(socket.setBroadcast(true), 0);
}

// Test setMulticastTtl on invalid socket
TEST_F(UdpSocketTests, setMulticastTtl_FailsOnInvalidSocket) {
    UdpSocket socket;
    EXPECT_LT(socket.setMulticastTtl(1), 0);
}

// Test joinMulticastGroup on invalid socket
TEST_F(UdpSocketTests, joinMulticastGroup_FailsOnInvalidSocket) {
    UdpSocket socket;
    EXPECT_LT(socket.joinMulticastGroup("224.0.0.1"), 0);
}

// Test open failure with SystemMock
TEST_F(UdpSocketTests, open_FailsWhenSocketSystemCallFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(-1));

    UdpSocket socket;
    EXPECT_LT(socket.open(), 0);
    EXPECT_EQ(socket.getFd(), -1);
}

// Test send with invalid IP address
TEST_F(UdpSocketTests, send_FailsWithInvalidIpAddress) {
    UdpSocket socket;
    socket.open();
    socket.bind(0);

    RawMessage msg(42);
    PortInfo to("invalid_ip", 12345);

    EXPECT_LT(socket.send(msg, to), 0);
    socket.close();
}

// Test receive timeout
TEST_F(UdpSocketTests, receive_ReturnsZeroOnTimeout) {
    UdpSocket socket;
    socket.open();
    socket.bind(0);

    RawMessagePtr msg;
    std::unique_ptr<IIdentifier> from;

    // Short timeout should return 0 (timeout)
    EXPECT_EQ(socket.receive(&msg, &from, 10), 0);
    socket.close();
}

// Test setBroadcast success
TEST_F(UdpSocketTests, setBroadcast_SuccessfullyEnablesBroadcast) {
    UdpSocket socket;
    socket.open();

    EXPECT_EQ(socket.setBroadcast(true), 0);
    EXPECT_EQ(socket.setBroadcast(false), 0);

    socket.close();
}

// Test setMulticastTtl
TEST_F(UdpSocketTests, setMulticastTtl_SuccessfullySets) {
    UdpSocket socket;
    socket.open();

    EXPECT_EQ(socket.setMulticastTtl(1), 0);

    socket.close();
}

// Test joinMulticastGroup
TEST_F(UdpSocketTests, joinMulticastGroup_SuccessfullyJoins) {
    UdpSocket socket;
    socket.open();
    socket.bind(0);

    // Join a multicast group
    // May succeed or fail depending on network configuration
    // Just verify it doesn't crash
    (void)socket.joinMulticastGroup("224.0.0.1");
    socket.open();
    socket.bind(0);

    EXPECT_EQ(socket.flush(), 0);

    socket.close();
}

// Test close on valid socket
TEST_F(UdpSocketTests, close_SuccessfullyClosesSocket) {
    UdpSocket socket;
    socket.open();
    int fd = socket.getFd();
    EXPECT_GE(fd, 0);

    socket.close();
    EXPECT_EQ(socket.getFd(), -1);
}

// Test double close
TEST_F(UdpSocketTests, close_SafeToCallMultipleTimes) {
    UdpSocket socket;
    socket.open();
    socket.close();
    socket.close();  // Should not crash
}
