
#include <stdio.h>
#include "gtest/gtest.h"
#include "PthreadMock.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcEndpointMock.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"

using namespace ::testing;

class LinxIpcServerTests : public testing::Test {
  public:
    std::shared_ptr<LinxMessageIpc> msg;
    std::shared_ptr<NiceMock<LinxIpcEndpointMock>> endpointMock;

    void SetUp() {
        endpointMock = std::make_shared<NiceMock<LinxIpcEndpointMock>>();
        msg = std::make_shared<LinxMessageIpc>(10);

        ON_CALL(*(endpointMock.get()), receive(_, _, _)).WillByDefault(Invoke([this]() {
            return msg;
        }));
    }
};

TEST_F(LinxIpcServerTests, handleMessage_CallQueueGet) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpointMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    EXPECT_CALL(*(endpointMock.get()), receive(10000, _, _));
    server->handleMessage(10000);
}

TEST_F(LinxIpcServerTests, handleMessage_ReturnZeroWhenReceiveNotRegisteredMessage) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpointMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    auto msg = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*(endpointMock.get()), receive(_, _, _)).WillOnce(Invoke([&msg]() {
        return msg;
    }));

    ASSERT_EQ(server->handleMessage(10000), 0);
}

TEST_F(LinxIpcServerTests, handleMessage_ReturnErrorWhenNotReceiveMessage) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpointMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    EXPECT_CALL(*(endpointMock.get()), receive(_, _, _)).WillOnce(Invoke([]() {
        return nullptr;
    }));

    ASSERT_EQ(server->handleMessage(10000), -1);
}

TEST_F(LinxIpcServerTests, handleMessage_ReturnCallbackResult) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpointMock);

    MockFunction<LinxIpcCallback> mockCallback;
    server->registerCallback(10, mockCallback.AsStdFunction(), nullptr);

    EXPECT_CALL(mockCallback, Call(msg.get(), nullptr)).WillOnce(Return(5));
    ASSERT_EQ(server->handleMessage(10000), 5);
}
