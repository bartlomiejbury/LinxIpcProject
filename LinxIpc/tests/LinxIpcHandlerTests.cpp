
#include "gtest/gtest.h"
#include "LinxIpc.h"
#include "LinxServerMock.h"
#include "IIdentifier.h"

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

