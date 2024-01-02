
#include <stdio.h>
#include "gtest/gtest.h"
#include "PthreadMock.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"

using namespace ::testing;

class LinxIpcServerTests : public testing::Test {
  public:
    NiceMock<PthreadMock> pthreadMock;
    NiceMock<LinxIpcSocketMock> *socketMock;
    NiceMock<LinxIpcQueueMock> *queueMock;

    void SetUp() {
        socketMock = new NiceMock<LinxIpcSocketMock>();
        queueMock = new NiceMock<LinxIpcQueueMock>();

        ON_CALL(pthreadMock, pthread_attr_init(_)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_attr_setinheritsched(_, _)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_attr_setschedpolicy(_, _)).WillByDefault(Return(0));

        ON_CALL(pthreadMock, pthread_attr_setschedparam(_, _)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_attr_setinheritsched(_, _)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_attr_destroy(_)).WillByDefault(Return(0));

        ON_CALL(pthreadMock, pthread_cancel(_)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_join(_, _)).WillByDefault(Return(0));

        ON_CALL(pthreadMock, pthread_create(_, _, _, _)).WillByDefault(Return(0));
    }
};

TEST_F(LinxIpcServerTests, endpoint_receiveNotRegistered) {
    auto server = std::make_shared<LinxIpcServerImpl>(queueMock, socketMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    auto msg = std::make_shared<LinxMessageIpc>(10);
    std::shared_ptr<LinxQueueContainer> container = std::make_shared<LinxQueueContainer>(msg, "TEST");

    EXPECT_CALL(*queueMock, get(10000, _, _)).WillOnce(Invoke([&container]() {
        return container;
    }));

    ASSERT_EQ(0, server->handleMessage(10000));
}

TEST_F(LinxIpcServerTests, endpoint_notReceiveMessage) {
    auto server = std::make_shared<LinxIpcServerImpl>(queueMock, socketMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    EXPECT_CALL(*queueMock, get(10000, _, _)).WillOnce(Invoke([]() {
        return nullptr;
    }));

    ASSERT_EQ(-1, server->handleMessage(10000));
}

TEST_F(LinxIpcServerTests, endpoint_receiveMessage) {
    auto server = std::make_shared<LinxIpcServerImpl>(queueMock, socketMock);

    MockFunction<LinxIpcCallback> mockCallback;

    server->registerCallback(10, mockCallback.AsStdFunction(), nullptr);

    auto msg = std::make_shared<LinxMessageIpc>(10);
    std::shared_ptr<LinxQueueContainer> container = std::make_shared<LinxQueueContainer>(msg, "TEST");

    EXPECT_CALL(*queueMock, get(10000, _, _)).WillOnce(Invoke([&container]() {
        return container;
    }));

    EXPECT_CALL(mockCallback, Call(msg.get(), nullptr)).WillOnce(Return(5));
    ASSERT_EQ(5, server->handleMessage(10000));
}
