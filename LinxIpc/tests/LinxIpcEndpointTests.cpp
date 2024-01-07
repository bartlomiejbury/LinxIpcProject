
#include <stdio.h>
#include "gtest/gtest.h"
#include "PthreadMock.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpc.h"
#include "LinxIpcEndpointImpl.h"

using namespace ::testing;

class LinxIpcEndpointTests : public testing::Test {
  public:
    NiceMock<PthreadMock> pthreadMock;
    NiceMock<LinxIpcSocketMock> *socketMock;
    NiceMock<LinxIpcQueueMock> *queueMock;

    void* (*callback)(void *arg){};
    void *data{};

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
        ON_CALL(*queueMock, get(_, _, _)).WillByDefault(Return(nullptr));

        ON_CALL(pthreadMock, pthread_create(_, _, _, _)).WillByDefault(DoAll(
            SaveArg<2>(&callback),
            SaveArg<3>(&data),
            Return(0)));
        }
};

TEST_F(LinxIpcEndpointTests, getPollFdCallQueueGetFd) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    EXPECT_CALL(*queueMock, getFd());

    endpoint->getPollFd();
}

TEST_F(LinxIpcEndpointTests, getPollFdReturnQueueGetFdResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    EXPECT_CALL(*queueMock, getFd()).WillOnce(Return(1));

    ASSERT_EQ(endpoint->getPollFd(), 1);
}

TEST_F(LinxIpcEndpointTests, sendCallSocketSend) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*socketMock, send(Ref(msg), "TEST"));

    endpoint->send(msg, client);
}

TEST_F(LinxIpcEndpointTests, sendReturnSocketSendResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*socketMock, send(_, _)).WillOnce(Return(1));

    ASSERT_EQ(endpoint->send(msg, client), 1);
}

TEST_F(LinxIpcEndpointTests, sendWithNoDestinationReturnError) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto msg = LinxMessageIpc(10);

    ASSERT_EQ(endpoint->send(msg, nullptr), -1);
}

TEST_F(LinxIpcEndpointTests, receive_callQueueWithNullOpt) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::nullopt;

    EXPECT_CALL(*queueMock, get(10000, Ref(sigsel), fromOpt));

    endpoint->receive(10000, sigsel);
}

TEST_F(LinxIpcEndpointTests, receive_callQueueWithClient) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::make_optional("TEST");

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*queueMock, get(10000, Ref(sigsel), fromOpt));

    endpoint->receive(10000, sigsel, client);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnNullWhenQueueReturnNull) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Return(nullptr));

    ASSERT_EQ(endpoint->receive(10000, sigsel), nullptr);
}

TEST_F(LinxIpcEndpointTests, endpoint_receiveMsg) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return std::make_shared<LinxQueueContainer>(msg, "TEST");
    }));

    ASSERT_EQ(endpoint->receive(10000, sigsel), msg);
    ASSERT_NE(msg->getClient(), nullptr);
    ASSERT_STREQ(msg->getClient()->getName().c_str(), "TEST");
}

TEST_F(LinxIpcEndpointTests, endpoint_createClientHasCorrectName) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    auto client = endpoint->createClient("TEST");
    ASSERT_STREQ(client->getName().c_str(), "TEST");
}

TEST_F(LinxIpcEndpointTests, start_CallStartNewThread) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _));
    endpoint->start();
}

TEST_F(LinxIpcEndpointTests, start_DoNothingWhenAlreadyStarted) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).Times(0);
    endpoint->start();
}

TEST_F(LinxIpcEndpointTests, stop_DoNothingWhenNotStarted) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    EXPECT_CALL(pthreadMock, pthread_cancel(_)).Times(0);
    endpoint->stop();
}

TEST_F(LinxIpcEndpointTests, stop_CallThreadCancel) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_cancel(_));
    endpoint->stop();
}

TEST_F(LinxIpcEndpointTests, thread_DoNothingWhenStopped) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).Times(0);
    EXPECT_CALL(*queueMock, add(_, _)).Times(0);

    endpoint->stop();
    callback(data);
}

TEST_F(LinxIpcEndpointTests, thread_NotAddToQueueWhenReceiveReturnNull) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(DoAll(
        Invoke([&endpoint](){endpoint->stop();}),
        Return(-1)));

    EXPECT_CALL(*queueMock, add(_, _)).Times(0);
    callback(data);
}

MATCHER_P(ClientMatcher, client, "") {
    const std::string &from = arg;
    return from == client;
}

MATCHER_P(MessageMatcher, reqId, "") {
    const LinxMessageIpc &message = arg;
    return message.getReqId() == reqId;
}

TEST_F(LinxIpcEndpointTests, thread_ReceiveHuntReqSendResponse) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(IPC_HUNT_REQ);
            *from = "TEST";
            endpoint->stop();
            return 0;
        }));

    EXPECT_CALL(*socketMock, send(MessageMatcher((uint32_t)IPC_HUNT_RSP), ClientMatcher("TEST")));
    EXPECT_CALL(*queueMock, add(_, _)).Times(0);

    callback(data);
}

TEST_F(LinxIpcEndpointTests, thread_ReceiveMsgAddToQueue) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            endpoint->stop();
            return 0;
        }));

    EXPECT_CALL(*queueMock, add(_, _))
        .With(AllArgs(Truly([](std::tuple<const std::shared_ptr<LinxMessageIpc>&, const std::string&> arg){
            const std::shared_ptr<LinxMessageIpc>&msg = std::get<0>(arg);
            const std::string &from = std::get<1>(arg);
            return msg->getReqId() == 10 && from == "TEST";
        })));

    callback(data);
}

TEST_F(LinxIpcEndpointTests, thread_ReceiveMsgAddToQueueError) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            endpoint->stop();
            return 0;
        }));

    EXPECT_CALL(*queueMock, add(Truly([](const LinxMessageIpcPtr &msg){ return msg->getReqId() == 10;}), "TEST"))
        .WillOnce(Return(-1));

    callback(data);
}
