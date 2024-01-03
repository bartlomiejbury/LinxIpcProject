
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

TEST_F(LinxIpcEndpointTests, endpoint_getFd) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    EXPECT_CALL(*queueMock, getFd()).WillOnce(Return(1));

    ASSERT_EQ(1, endpoint->getPollFd());
}

TEST_F(LinxIpcEndpointTests, endpoint_getQueueSize) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    EXPECT_CALL(*queueMock, size()).WillOnce(Return(1));

    ASSERT_EQ(1, endpoint->getQueueSize());
}

TEST_F(LinxIpcEndpointTests, endpoint_send) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    EXPECT_CALL(*(client.get()), getName()).WillOnce(Return("TEST"));
    EXPECT_CALL(*socketMock, send(Ref(msg), "TEST")).WillOnce(Return(1));

    ASSERT_EQ(1, endpoint->send(msg, client));
}

TEST_F(LinxIpcEndpointTests, endpoint_send_noDestination) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto msg = LinxMessageIpc(10);

    ASSERT_EQ(-1, endpoint->send(msg, nullptr));
}

TEST_F(LinxIpcEndpointTests, endpoint_receive_callQueueWithNullOpt) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::nullopt;

    EXPECT_CALL(*queueMock, get(10000, Ref(sigsel), fromOpt)).WillOnce(Return(nullptr));

    endpoint->receive(10000, sigsel);
}

TEST_F(LinxIpcEndpointTests, endpoint_receive_callQueueWithCLient) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::make_optional("TEST");

    EXPECT_CALL(*(client.get()), getName()).WillOnce(Return("TEST"));
    EXPECT_CALL(*queueMock, get(10000, Ref(sigsel), fromOpt)).WillOnce(Return(nullptr));

    endpoint->receive(10000, sigsel, client);
}

TEST_F(LinxIpcEndpointTests, endpoint_receive_queueGetNullPtr) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Return(nullptr));

    ASSERT_EQ(nullptr, endpoint->receive(10000, sigsel));
}

TEST_F(LinxIpcEndpointTests, endpoint_receiveMsg) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    auto msg = std::make_shared<LinxMessageIpc>(10);
    std::shared_ptr<LinxQueueContainer> container = std::make_shared<LinxQueueContainer>(msg, "TEST");

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&container]() {
        return container;
    }));

    ASSERT_EQ(msg, endpoint->receive(10000, sigsel));
    ASSERT_NE(nullptr, msg->getClient());
    ASSERT_STREQ("TEST", msg->getClient()->getName().c_str());
}

TEST_F(LinxIpcEndpointTests, endpoint_createClient) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    auto client = endpoint->createClient("TEST");
    ASSERT_STREQ("TEST", client->getName().c_str());
}

TEST_F(LinxIpcEndpointTests, endpoint_startNewThread) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).WillOnce(Return(0));
    endpoint->start();
}

TEST_F(LinxIpcEndpointTests, endpoint_startSecondTime) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).Times(0);
    endpoint->start();
}

TEST_F(LinxIpcEndpointTests, endpoint_stopWhenNotStarted) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    EXPECT_CALL(pthreadMock, pthread_cancel(_)).Times(0);
    endpoint->stop();
}

TEST_F(LinxIpcEndpointTests, endpoint_stopKillThread) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_cancel(_)).WillOnce(Return(0));
    endpoint->stop();
}

TEST_F(LinxIpcEndpointTests, endpoint_thread_Stopped) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    void* (*callback)(void *arg){};
    void *data{};

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).WillOnce(DoAll(
        SaveArg<2>(&callback),
        SaveArg<3>(&data),
        Return(0)));

    endpoint->start();

    //EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*socketMock, receive(_, _, _)).Times(0);
    EXPECT_CALL(*queueMock, add(_, _)).Times(0);

    endpoint->stop();
    callback(data);
}

TEST_F(LinxIpcEndpointTests, endpoint_thread_NotReceived) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    void* (*callback)(void *arg){};
    void *data{};

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).WillOnce(DoAll(
        SaveArg<2>(&callback),
        SaveArg<3>(&data),
        Return(0)));

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

TEST_F(LinxIpcEndpointTests, endpoint_thread_ReceiveHuntReq) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    void* (*callback)(void *arg){};
    void *data{};

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).WillOnce(DoAll(
        SaveArg<2>(&callback),
        SaveArg<3>(&data),
        Return(0)));

    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(IPC_HUNT_REQ);
            *from = "TEST";
            endpoint->stop();
            return 0;
        }));

    EXPECT_CALL(*socketMock, send(MessageMatcher((uint32_t)IPC_HUNT_RSP), ClientMatcher("TEST"))).Times(1);
    EXPECT_CALL(*queueMock, add(_, _)).Times(0);

    callback(data);
}

TEST_F(LinxIpcEndpointTests, endpoint_thread_ReceiveMsg) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    void* (*callback)(void *arg){};
    void *data{};

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).WillOnce(DoAll(
        SaveArg<2>(&callback),
        SaveArg<3>(&data),
        Return(0)));

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
        })))
        .WillOnce(Return(0));

    callback(data);
}

TEST_F(LinxIpcEndpointTests, endpoint_thread_ReceiveMsgAddError) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(queueMock, socketMock);

    void* (*callback)(void *arg){};
    void *data{};

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).WillOnce(DoAll(
        SaveArg<2>(&callback),
        SaveArg<3>(&data),
        Return(0)));

    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            endpoint->stop();
            return 0;
        }));

    EXPECT_CALL(*queueMock, add(Truly([](const LinxMessageIpcPtr &msg){ return msg->getReqId() == 10;}), "TEST")).WillOnce(Return(-1));

    callback(data);
}
