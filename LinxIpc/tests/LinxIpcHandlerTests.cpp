
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcServerMock.h"
#include "LinxIpc.h"
#include "LinxIpcHandlerImpl.h"
#include "LinxIpcPrivate.h"

using namespace ::testing;

class LinxIpcHandlerTests : public testing::Test {
};

TEST_F(LinxIpcHandlerTests, handleMessage_ReturnZeroWhenReceiveNotRegisteredMessage) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };

    auto server = std::make_shared<NiceMock<LinxIpcServerMock>>();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*server, receive(_, _, _)).WillOnce(Return(msg));
    ASSERT_EQ(handler->handleMessage(10000), 0);
}

TEST_F(LinxIpcHandlerTests, handleMessage_ReturnErrorWhenNotReceiveMessage) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };

    auto server = std::make_shared<NiceMock<LinxIpcServerMock>>();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*server, receive(_, _, _)).WillOnce(Return(nullptr));   

    ASSERT_EQ(handler->handleMessage(10000), -1);
}

TEST_F(LinxIpcHandlerTests, handleMessage_ReturnCallbackResult) {
    MockFunction<LinxIpcCallback> mockCallback;
    std::map<uint32_t, IpcContainer> handlers {
        {10, {mockCallback.AsStdFunction(), nullptr}}
    };

    auto server = std::make_shared<NiceMock<LinxIpcServerMock>>();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*server, receive(_, _, _)).WillOnce(Return(msg));   
    EXPECT_CALL(mockCallback, Call(msg.get(), nullptr)).WillOnce(Return(5));
    ASSERT_EQ(handler->handleMessage(10000), 5);
}

TEST_F(LinxIpcHandlerTests, startStop) {
    MockFunction<LinxIpcCallback> mockCallback;
    std::map<uint32_t, IpcContainer> handlers {
        {10, {mockCallback.AsStdFunction(), nullptr}}
    };

    auto server = std::make_shared<NiceMock<LinxIpcServerMock>>();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*server, start());
    EXPECT_CALL(*server, stop()).Times(2);

    handler->start();
    handler->stop();
}

TEST_F(LinxIpcHandlerTests, getPollFd) {
    MockFunction<LinxIpcCallback> mockCallback;
    std::map<uint32_t, IpcContainer> handlers {
        {10, {mockCallback.AsStdFunction(), nullptr}}
    };

    auto server = std::make_shared<NiceMock<LinxIpcServerMock>>();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*server, getPollFd()).WillOnce(Return(42));
    ASSERT_EQ(handler->getPollFd(), 42);
}

TEST_F(LinxIpcHandlerTests, send) {
    MockFunction<LinxIpcCallback> mockCallback;
    std::map<uint32_t, IpcContainer> handlers {
        {10, {mockCallback.AsStdFunction(), nullptr}}
    };

    auto server = std::make_shared<NiceMock<LinxIpcServerMock>>();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*server, send(Ref(msg), _)).WillOnce(Return(42));
    ASSERT_EQ(handler->send(msg, nullptr), 42);
}

TEST_F(LinxIpcHandlerTests, createClient) {
    MockFunction<LinxIpcCallback> mockCallback;
    std::map<uint32_t, IpcContainer> handlers {
        {10, {mockCallback.AsStdFunction(), nullptr}}
    };

    auto server = std::make_shared<NiceMock<LinxIpcServerMock>>();
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*server, createClient("TEST")).WillOnce(Return(nullptr));
    ASSERT_EQ(handler->createClient("TEST"), nullptr);
}