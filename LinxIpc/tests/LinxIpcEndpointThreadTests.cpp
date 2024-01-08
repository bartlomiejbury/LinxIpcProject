
#include <stdio.h>
#include "gtest/gtest.h"
#include "PthreadMock.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpcEndpointMock.h"
#include "LinxIpc.h"
#include "LinxIpcEndpointThreadImpl.h"

using namespace ::testing;

class LinxIpcEndpointThreadTests : public testing::Test {
  public:
    NiceMock<PthreadMock> pthreadMock;
    NiceMock<LinxIpcQueueMock> *queueMock;
    std::shared_ptr<NiceMock<LinxIpcEndpointMock>> endpointMock;

    void* (*callback)(void *arg){};
    void *data{};

    void SetUp() {
        endpointMock = std::make_shared<NiceMock<LinxIpcEndpointMock>>();
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
        ON_CALL(*queueMock, get(_, _, _)).WillByDefault(Return(nullptr));

        ON_CALL(pthreadMock, pthread_create(_, _, _, _)).WillByDefault(DoAll(
            SaveArg<2>(&callback),
            SaveArg<3>(&data),
            Return(0)));
        }
};

TEST_F(LinxIpcEndpointThreadTests, getPollFdCallQueueGetFd) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    EXPECT_CALL(*queueMock, getFd());

    endpoint->getPollFd();
}

TEST_F(LinxIpcEndpointThreadTests, getPollFdReturnQueueGetFdResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    EXPECT_CALL(*queueMock, getFd()).WillOnce(Return(1));

    ASSERT_EQ(endpoint->getPollFd(), 1);
}

TEST_F(LinxIpcEndpointThreadTests, sendCallEndpointSend) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    EXPECT_CALL(*(endpointMock.get()), send(Ref(msg), Ref(client)));
    endpoint->send(msg, client);
}

TEST_F(LinxIpcEndpointThreadTests, sendReturnSocketSendResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    EXPECT_CALL(*(endpointMock.get()), send(_, _)).WillOnce(Return(1));
    ASSERT_EQ(endpoint->send(msg, client), 1);
}

MATCHER_P(ClientMatcher, client, "") {
    const std::shared_ptr<LinxIpcClient> &from = arg;
    return from.get() == client.get();
}

MATCHER_P(SigselMatcher, signals, "") {
    const std::vector<uint32_t> &sigsel = arg;
    const std::vector<uint32_t> expected = signals;
    return sigsel == expected;
}

TEST_F(LinxIpcEndpointThreadTests, receive_callQueueWithClient) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    auto sigsel = std::initializer_list<uint32_t>{4};

    EXPECT_CALL(*queueMock, get(10000, SigselMatcher(sigsel), ClientMatcher(client)));
    endpoint->receive(10000, sigsel, client);
}

TEST_F(LinxIpcEndpointThreadTests, receive_ReturnNullWhenQueueReturnNull) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Return(nullptr));

    ASSERT_EQ(endpoint->receive(10000, sigsel), nullptr);
}

TEST_F(LinxIpcEndpointThreadTests, endpoint_receiveMsg) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return msg;
    }));

    ASSERT_EQ(endpoint->receive(10000, sigsel), msg);
}

TEST_F(LinxIpcEndpointThreadTests, endpoint_createClientCallEndpointCreateCLient) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);

    EXPECT_CALL(*(endpointMock.get()), createClient("TEST"));
    auto client = endpoint->createClient("TEST");
}

TEST_F(LinxIpcEndpointThreadTests, start_CallStartNewThread) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _));
    endpoint->start();
}

TEST_F(LinxIpcEndpointThreadTests, start_DoNothingWhenAlreadyStarted) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).Times(0);
    endpoint->start();
}

TEST_F(LinxIpcEndpointThreadTests, stop_DoNothingWhenNotStarted) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);

    EXPECT_CALL(pthreadMock, pthread_cancel(_)).Times(0);
    endpoint->stop();
}

TEST_F(LinxIpcEndpointThreadTests, stop_CallThreadCancel) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_cancel(_));
    endpoint->stop();
}

TEST_F(LinxIpcEndpointThreadTests, thread_DoNothingWhenStopped) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*(endpointMock.get()), receive(_, _, _)).Times(0);
    EXPECT_CALL(*queueMock, add(_)).Times(0);

    endpoint->stop();
    callback(data);
}

TEST_F(LinxIpcEndpointThreadTests, thread_NotAddToQueueWhenReceiveReturnNull) {
    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*(endpointMock.get()), receive(_, _, _)).WillOnce(DoAll(
        Invoke([&endpoint](){endpoint->stop();}),
        Return(nullptr)));

    EXPECT_CALL(*queueMock, add(_)).Times(0);
    callback(data);
}

MATCHER_P(MessageMatcher, reqId, "") {
    const LinxMessageIpc &message = arg;
    return message.getReqId() == reqId;
}

TEST_F(LinxIpcEndpointThreadTests, thread_ReceiveHuntReqSendResponse) {
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*(endpointMock.get()), receive(_, _, _)).WillOnce(
        Invoke([&endpoint, &client](){
            endpoint->stop();
            auto msg = std::make_shared<LinxMessageIpc>(IPC_HUNT_REQ);
            msg->setClient(client);
            return msg;
        }));

    EXPECT_CALL(*(client.get()), send(MessageMatcher((uint32_t)IPC_HUNT_RSP)));
    EXPECT_CALL(*queueMock, add(_)).Times(0);

    callback(data);
}

TEST_F(LinxIpcEndpointThreadTests, thread_ReceiveMsgAddToQueue) {
    auto msg = std::make_shared<LinxMessageIpc>(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    msg->setClient(client);

    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    endpoint->start();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*(endpointMock.get()), receive(_, _, _)).WillOnce(
        Invoke([&endpoint, &msg](){
            endpoint->stop();
            return msg;
        }));

    EXPECT_CALL(*queueMock, add(msg));
    callback(data);
}

TEST_F(LinxIpcEndpointThreadTests, thread_ReceiveMsgAddToQueueError) {
    auto msg = std::make_shared<LinxMessageIpc>(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    msg->setClient(client);

    auto endpoint = std::make_shared<LinxIpcEndpointThreadImpl>(endpointMock, queueMock);
    endpoint->start();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*(endpointMock.get()), receive(_, _, _)).WillOnce(
        Invoke([&endpoint, &msg](){
            endpoint->stop();
            return msg;
        }));

    EXPECT_CALL(*queueMock, add(msg)).WillOnce(Return(-1));
    callback(data);
}
