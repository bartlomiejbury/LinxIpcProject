
#include "gtest/gtest.h"
#include "LinxIpc.h"
#include "LinxServerMock.h"
#include "IIdentifier.h"
#include "UnixLinx.h"

using namespace ::testing;

class LinxIpcHandlerTests : public testing::Test {
};

TEST_F(LinxIpcHandlerTests, handleMessage_ReturnZeroWhenReceiveNotRegisteredMessage) {
    IpcContainer cb {
        [](const LinxReceivedMessageSharedPtr &msg, void *data) { return 0; }, nullptr
    };

    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);
    handler.registerCallback(5, cb.callback, cb.data);

    auto msg = std::make_shared<LinxReceivedMessage>();
    msg->message = std::make_unique<RawMessage>(10);
    msg->from = nullptr;

    EXPECT_CALL(*server, receive(_, _, _)).WillOnce(Return(msg));
    ASSERT_EQ(handler.handleMessage(10000), 0);
}

TEST_F(LinxIpcHandlerTests, handleMessage_ReturnErrorWhenNotReceiveMessage) {
    IpcContainer cb {
        [](const LinxReceivedMessageSharedPtr &msg, void *data) { return 0; }, nullptr
    };

    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);
    handler.registerCallback(5, cb.callback, cb.data);

    EXPECT_CALL(*server, receive(_, _, _)).WillOnce(Return(nullptr));
    ASSERT_EQ(handler.handleMessage(10000), -1);
}

TEST_F(LinxIpcHandlerTests, handleMessage_ReturnCallbackResult) {
    MockFunction<LinxIpcCallback> mockCallback;

    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);
    handler.registerCallback(10, mockCallback.AsStdFunction(), nullptr);

    auto msg = std::make_shared<LinxReceivedMessage>();
    msg->message = std::make_unique<RawMessage>(10);
    msg->from = nullptr;

    EXPECT_CALL(*server, receive(_, _, _)).WillOnce(Return(msg));
    EXPECT_CALL(mockCallback, Call(msg, nullptr)).WillOnce(Return(5));
    ASSERT_EQ(handler.handleMessage(10000), 5);
}

TEST_F(LinxIpcHandlerTests, startStop) {
    MockFunction<LinxIpcCallback> mockCallback;

    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);
    handler.registerCallback(10, mockCallback.AsStdFunction(), nullptr);

    EXPECT_CALL(*server, start());
    EXPECT_CALL(*server, stop()).Times(2);

    handler.start();
    handler.stop();
}

TEST_F(LinxIpcHandlerTests, getPollFd) {
    MockFunction<LinxIpcCallback> mockCallback;

    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);
    handler.registerCallback(10, mockCallback.AsStdFunction(), nullptr);

    EXPECT_CALL(*server, getPollFd()).WillOnce(Return(42));
    ASSERT_EQ(handler.getPollFd(), 42);
}

TEST_F(LinxIpcHandlerTests, getName) {
    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);

    EXPECT_CALL(*server, getName()).WillOnce(Return("TestServer"));
    ASSERT_EQ(handler.getName(), "TestServer");
}

TEST_F(LinxIpcHandlerTests, send) {
    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);

    RawMessage message(42);
    UnixInfo to("destination");

    EXPECT_CALL(*server, send(_, _)).WillOnce(Return(100));
    ASSERT_EQ(handler.send(message, to), 100);
}

TEST_F(LinxIpcHandlerTests, receive) {
    auto server = std::make_shared<NiceMock<LinxServerMock>>();
    auto handler = LinxIpcHandler(server);

    auto msg = std::make_shared<LinxReceivedMessage>();
    msg->message = std::make_unique<RawMessage>(10);
    msg->from = std::make_unique<UnixInfo>("sender");

    EXPECT_CALL(*server, receive(1000, _, _)).WillOnce(Return(msg));
    auto result = handler.receive(1000, LINX_ANY_SIG, LINX_ANY_FROM);
    ASSERT_EQ(result, msg);
}

TEST_F(LinxIpcHandlerTests, sendResponse_Success) {
    auto server = std::make_shared<NiceMock<LinxServerMock>>();

    auto msg = std::make_shared<LinxReceivedMessage>();
    msg->message = std::make_unique<RawMessage>(10);
    msg->from = std::make_unique<UnixInfo>("sender");
    msg->server = server;

    RawMessage response(20);

    EXPECT_CALL(*server, send(_, _)).WillOnce(Return(50));
    ASSERT_EQ(msg->sendResponse(response), 50);
}

TEST_F(LinxIpcHandlerTests, sendResponse_FailureWhenServerExpired) {
    auto server = std::make_shared<NiceMock<LinxServerMock>>();

    auto msg = std::make_shared<LinxReceivedMessage>();
    msg->message = std::make_unique<RawMessage>(10);
    msg->from = std::make_unique<UnixInfo>("sender");
    msg->server = server;

    // Let the server expire by resetting the shared_ptr
    server.reset();

    RawMessage response(20);

    // Should return -1 when server is expired
    ASSERT_EQ(msg->sendResponse(response), -1);
}

