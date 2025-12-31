#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "AfUnixSocket.h"
#include "RawMessage.h"
#include "SystemMock.h"
#include <sys/socket.h>
#include <sys/un.h>

using namespace ::testing;

class AfUnixSocketTests : public testing::Test {
  protected:
    void SetUp() override {
    }

    void TearDown() override {
        // Clean up any test socket files
        unlink("/tmp/test_socket_12345");
        unlink("/tmp/test_socket_67890");
    }
};

// Test constructor
TEST_F(AfUnixSocketTests, constructor_InitializesWithName) {
    AfUnixSocket socket("test_socket");
    EXPECT_EQ(socket.getFd(), -1);
}

// Test open success
TEST_F(AfUnixSocketTests, open_SuccessfullyCreatesSocket) {
    AfUnixSocket socket("test_socket_12345");
    EXPECT_EQ(socket.open(), 0);
    EXPECT_GE(socket.getFd(), 0);
    socket.close();
}

// Test open when already open
TEST_F(AfUnixSocketTests, open_FailsWhenAlreadyOpen) {
    AfUnixSocket socket("test_socket_12345");
    socket.open();

    EXPECT_LT(socket.open(), 0);

    socket.close();
}

// Test close
TEST_F(AfUnixSocketTests, close_SuccessfullyClosesSocket) {
    AfUnixSocket socket("test_socket_12345");
    socket.open();
    int fd = socket.getFd();
    EXPECT_GE(fd, 0);

    socket.close();
    EXPECT_EQ(socket.getFd(), -1);
}

// Test double close
TEST_F(AfUnixSocketTests, close_SafeToCallMultipleTimes) {
    AfUnixSocket socket("test_socket_12345");
    socket.open();
    socket.close();
    socket.close();  // Should not crash
}

// Test receive on invalid socket
TEST_F(AfUnixSocketTests, receive_FailsOnInvalidSocket) {
    AfUnixSocket socket("test_socket");
    RawMessagePtr msg;
    StringIdentifier from;

    EXPECT_LT(socket.receive(&msg, &from, 10), 0);
}

// Test send on invalid socket
TEST_F(AfUnixSocketTests, send_FailsOnInvalidSocket) {
    AfUnixSocket socket("test_socket");
    RawMessage msg(42);
    StringIdentifier to("other_socket");

    EXPECT_LT(socket.send(msg, to), 0);
}

// Test flush on invalid socket
TEST_F(AfUnixSocketTests, flush_FailsOnInvalidSocket) {
    AfUnixSocket socket("test_socket");
    EXPECT_LT(socket.flush(), 0);
}

// Test receive timeout
TEST_F(AfUnixSocketTests, receive_ReturnsZeroOnTimeout) {
    AfUnixSocket socket("test_socket_12345");
    socket.open();

    RawMessagePtr msg;
    StringIdentifier from;

    // Short timeout should return 0 (timeout)
    EXPECT_EQ(socket.receive(&msg, &from, 10), 0);

    socket.close();
}

// Test flush on valid socket
TEST_F(AfUnixSocketTests, flush_ReturnsZeroWhenNoData) {
    AfUnixSocket socket("test_socket_12345");
    socket.open();

    EXPECT_EQ(socket.flush(), 0);

    socket.close();
}

// Test open failure - socket system call fails
TEST_F(AfUnixSocketTests, open_FailsWhenSocketSystemCallFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_UNIX, SOCK_DGRAM, 0))
        .WillOnce(Return(-1));

    AfUnixSocket socket("test_socket");
    EXPECT_LT(socket.open(), 0);
    EXPECT_EQ(socket.getFd(), -1);
}

// Test open failure - bind fails
TEST_F(AfUnixSocketTests, open_FailsWhenBindFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_UNIX, SOCK_DGRAM, 0))
        .WillOnce(Return(100));
    EXPECT_CALL(mock, bind(100, _, _))
        .WillOnce(Return(-1));
    EXPECT_CALL(mock, close(100))
        .WillOnce(Return(0));

    AfUnixSocket socket("test_socket");
    EXPECT_LT(socket.open(), 0);
    EXPECT_EQ(socket.getFd(), -1);
}

// Test getFd
TEST_F(AfUnixSocketTests, getFd_ReturnsCorrectFd) {
    AfUnixSocket socket("test_socket_12345");
    EXPECT_EQ(socket.getFd(), -1);

    socket.open();
    EXPECT_GE(socket.getFd(), 0);

    socket.close();
    EXPECT_EQ(socket.getFd(), -1);
}

// Test destructor closes socket
TEST_F(AfUnixSocketTests, destructor_ClosesSocket) {
    {
        AfUnixSocket socket("test_socket_12345");
        socket.open();
        EXPECT_GE(socket.getFd(), 0);
    }
    // Socket should be closed by destructor
    // Verify by trying to create a new socket with the same name
    AfUnixSocket socket2("test_socket_12345");
    EXPECT_EQ(socket2.open(), 0);
    socket2.close();
}

// Test send with unconnected destination
TEST_F(AfUnixSocketTests, send_FailsWhenDestinationDoesNotExist) {
    AfUnixSocket socket("test_socket_12345");
    socket.open();

    RawMessage msg(42);
    StringIdentifier to("nonexistent_socket");

    // Should fail because destination doesn't exist
    EXPECT_LT(socket.send(msg, to), 0);

    socket.close();
}
