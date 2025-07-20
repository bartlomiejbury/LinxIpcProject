
#include <stdio.h>
#include "gtest/gtest.h"
#include "PthreadMock.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"

using namespace ::testing;

class LinxIpcSimpleServerTests : public testing::Test {
  public:
    NiceMock<LinxIpcSocketMock> *socketMock;
    
    void SetUp() {
        socketMock = new NiceMock<LinxIpcSocketMock>();

        ON_CALL(*socketMock, receive(_, _, _)).WillByDefault(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));
    }
};

TEST_F(LinxIpcSimpleServerTests, getPollFdCallSocketGetFd) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    EXPECT_CALL(*socketMock, getFd());

    server->getPollFd();
}

TEST_F(LinxIpcSimpleServerTests, getPollFdReturnSocketGetFdResult) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    EXPECT_CALL(*socketMock, getFd()).WillOnce(Return(1));

    ASSERT_EQ(server->getPollFd(), 1);
}

TEST_F(LinxIpcSimpleServerTests, sendReturnSocketSendResult) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*socketMock, send(_, _)).WillOnce(Return(1));

    ASSERT_EQ(server->send(msg, client), 1);
}

TEST_F(LinxIpcSimpleServerTests, sendWithNoDestinationReturnError) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto msg = LinxMessageIpc(10);

    ASSERT_EQ(server->send(msg, nullptr), -1);
}

TEST_F(LinxIpcSimpleServerTests, endpoint_createClientHasCorrectName) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);

    auto client = server->createClient("TEST");
    ASSERT_STREQ(client->getName().c_str(), "TEST");
}

TEST_F(LinxIpcSimpleServerTests, receive_callSocketSend) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::nullopt;

    EXPECT_CALL(*socketMock, receive(_, _, 10000));
    server->receive(10000, sigsel);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnNullWhenSocketReturnError) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(Return(-1));

    ASSERT_EQ(server->receive(10000, sigsel), nullptr);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnMsgWhenSignalMatchAny) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(server->receive(10000, LINX_ANY_SIG, LINX_ANY_FROM), message);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnNullWhenSignalNotMatch) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(server->receive(10000, sigsel, LINX_ANY_FROM), nullptr);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnMsgWhenSignalMatch) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{10};
    auto message = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(server->receive(10000, sigsel), message);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnNullWhenClientNotMatch) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);

    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*(clientMock.get()), getName()).WillOnce(Return("TEST2"));
    ASSERT_EQ(server->receive(10000, LINX_ANY_SIG, clientMock), nullptr);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnMsgWhenClientMatch) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);

    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*(clientMock.get()), getName()).WillOnce(Return("TEST"));
    ASSERT_EQ(server->receive(10000, LINX_ANY_SIG, clientMock), message);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnMsgWhenSignalClientMatch) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);
    auto sigsel = std::initializer_list<uint32_t>{10};
    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*(clientMock.get()), getName()).WillOnce(Return("TEST"));
    ASSERT_EQ(server->receive(10000, sigsel, clientMock), message);
    ASSERT_NE(message->getClient(), nullptr);
    ASSERT_STREQ(message->getClient()->getName().c_str(), "TEST");
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_CallSocketReceive) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    EXPECT_CALL(*socketMock, receive(_, _, 10000));
    server->handleMessage(10000);
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_ReturnZeroWhenReceiveNotRegisteredMessage) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    auto message = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    ASSERT_EQ(server->handleMessage(10000), 0);
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_ReturnErrorWhenNotReceiveMessage) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            return -1;
        }));

    ASSERT_EQ(server->handleMessage(10000), -1);
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_ReturnCallbackResult) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);

    MockFunction<LinxIpcCallback> mockCallback;
    server->registerCallback(10, mockCallback.AsStdFunction(), (void*)5);

    auto message = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(mockCallback, Call(message.get(), (void*)5)).WillOnce(Return(5));
    ASSERT_EQ(server->handleMessage(10000), 5);
}

MATCHER_P(MessageMatcher, reqId, "") {
    const LinxMessageIpc &message = arg;
    return message.getReqId() == reqId;
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_ReceiveHuntReqSendResponse) {

    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(IPC_HUNT_REQ);
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*socketMock, send(MessageMatcher((uint32_t)IPC_HUNT_RSP), "TEST"));
    ASSERT_EQ(server->handleMessage(10000), 0);
}

class LinxIpcExtendedServerTests : public testing::Test {
  public:
    NiceMock<PthreadMock> pthreadMock;
    NiceMock<LinxIpcQueueMock> *queueMock;
    NiceMock<LinxIpcSocketMock> *socketMock;

    void* (*callback)(void *arg){};
    void *data{};

    void SetUp() {
        socketMock = new NiceMock<LinxIpcSocketMock>();
        queueMock = new NiceMock<LinxIpcQueueMock>();

        ON_CALL(*socketMock, receive(_, _, _)).WillByDefault(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));

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

TEST_F(LinxIpcExtendedServerTests, getPollFdCallQueueGetFd) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    EXPECT_CALL(*queueMock, getFd());

    endpoint->getPollFd();
}

TEST_F(LinxIpcExtendedServerTests, getPollFdReturnQueueGetFdResult) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    EXPECT_CALL(*queueMock, getFd()).WillOnce(Return(1));

    ASSERT_EQ(endpoint->getPollFd(), 1);
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

TEST_F(LinxIpcExtendedServerTests, receive_callQueueWithClient) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::make_optional("TEST");

    EXPECT_CALL(*(client.get()), getName()).WillOnce(Return("TEST"));
    EXPECT_CALL(*queueMock, get(10000, SigselMatcher(sigsel), fromOpt));
    endpoint->receive(10000, sigsel, client);
}

TEST_F(LinxIpcExtendedServerTests, receive_callQueueWithNullOpt) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::nullopt;

    EXPECT_CALL(*queueMock, get(10000, SigselMatcher(sigsel), fromOpt));
    endpoint->receive(10000, sigsel);
}

TEST_F(LinxIpcExtendedServerTests, receive_ReturnNullWhenQueueReturnNull) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Return(nullptr));

    ASSERT_EQ(endpoint->receive(10000, sigsel), nullptr);
}

TEST_F(LinxIpcExtendedServerTests, endpoint_receiveMsg) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto sigsel = std::initializer_list<uint32_t>{4};

    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("TEST"));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(client);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return LinxQueueElement(new std::pair<LinxMessageIpcPtr, std::string>(msg, "TEST"));
    }));

    ASSERT_EQ(endpoint->receive(10000, sigsel), msg);
    ASSERT_NE(msg->getClient(), nullptr);
    ASSERT_STREQ(msg->getClient()->getName().c_str(), "TEST");
}

TEST_F(LinxIpcExtendedServerTests, start_CallStartNewThread) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _));
    endpoint->start();
}

TEST_F(LinxIpcExtendedServerTests, start_DoNothingWhenAlreadyStarted) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_create(_, _, _, _)).Times(0);
    endpoint->start();
}

TEST_F(LinxIpcExtendedServerTests, stop_DoNothingWhenNotStarted) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);

    EXPECT_CALL(pthreadMock, pthread_cancel(_)).Times(0);
    endpoint->stop();
}

TEST_F(LinxIpcExtendedServerTests, stop_CallThreadCancel) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(pthreadMock, pthread_cancel(_));
    endpoint->stop();
}

TEST_F(LinxIpcExtendedServerTests, thread_DoNothingWhenStopped) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).Times(0);
    EXPECT_CALL(*queueMock, add(_, _)).Times(0);

    endpoint->stop();
    callback(data);
}

TEST_F(LinxIpcExtendedServerTests, thread_NotAddToQueueWhenReceiveReturnError) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(DoAll(
        Invoke([&endpoint](){endpoint->stop();}),
        Return(-1)));

    EXPECT_CALL(*queueMock, add(_, _)).Times(0);
    callback(data);
}

TEST_F(LinxIpcExtendedServerTests, thread_ReceiveHuntReqSendResponse) {

    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            endpoint->stop();
            *msg = std::make_shared<LinxMessageIpc>(IPC_HUNT_REQ);
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*socketMock, send(MessageMatcher((uint32_t)IPC_HUNT_RSP), "TEST"));
    EXPECT_CALL(*queueMock, add(_, _)).Times(0);

    callback(data);
}

TEST_F(LinxIpcExtendedServerTests, thread_ReceiveMsgAddToQueue) {
    auto msg = std::make_shared<LinxMessageIpc>(10);

    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint, &msg](LinxMessageIpcPtr *message, std::string *from, int timeout){
            endpoint->stop();
            *message = msg;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*queueMock, add(std::move(msg), "TEST"));
    
    callback(data);
    testing::Mock::VerifyAndClearExpectations(queueMock);
}

TEST_F(LinxIpcExtendedServerTests, thread_ReceiveMsgAddToQueueError) {
    auto msg = std::make_shared<LinxMessageIpc>(10);

    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint, &msg](LinxMessageIpcPtr *message, std::string *from, int timeout){
            endpoint->stop();
            *message = msg;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*queueMock, add(_, _)).WillOnce(Return(-1));
    callback(data);
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_CallQueueGet) {
    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    EXPECT_CALL(*queueMock, get(10000, _, _));
    server->handleMessage(10000);
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_ReturnZeroWhenReceiveNotRegisteredMessage) {
    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("TEST"));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(client);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return LinxQueueElement(new std::pair<LinxMessageIpcPtr, std::string>(msg, "TEST"));
    }));

    ASSERT_EQ(server->handleMessage(10000), 0);
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_ReturnErrorWhenNotReceiveMessage) {
    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    server->registerCallback(5, [](LinxMessageIpc *msg, void *data) { return 0;}, nullptr);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([]() {
        return nullptr;
    }));

    ASSERT_EQ(server->handleMessage(10000), -1);
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_ReturnCallbackResult) {
    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);

    MockFunction<LinxIpcCallback> mockCallback;
    server->registerCallback(10, mockCallback.AsStdFunction(), nullptr);

    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("TEST"));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(client);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return LinxQueueElement(new std::pair<LinxMessageIpcPtr, std::string>(msg, "TEST"));
    }));

    EXPECT_CALL(mockCallback, Call(msg.get(), nullptr)).WillOnce(Return(5));
    ASSERT_EQ(server->handleMessage(10000), 5);
}
