
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcPrivate.h"

using namespace ::testing;

class LinxIpcExtendedServerImplTask: public LinxIpcExtendedServerImpl {
    public:
        LinxIpcExtendedServerImplTask(LinxIpcSocket *socket, LinxQueue *queue)
            : LinxIpcExtendedServerImpl(socket, queue) {}

        void task() {
            LinxIpcExtendedServerImpl::task();
        }

        void start() {
            running = true;
        }

        void stop() {
            running = false;
        }
};

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
    ON_CALL(*(clientMock.get()), getName()).WillByDefault(Return("TEST2"));

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));

    ASSERT_EQ(server->receive(10000, LINX_ANY_SIG, clientMock), nullptr);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnMsgWhenClientMatch) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);

    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*(clientMock.get()), getName()).WillByDefault(Return("TEST"));

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    ASSERT_EQ(server->receive(10000, LINX_ANY_SIG, clientMock), message);
}

TEST_F(LinxIpcSimpleServerTests, receive_ReturnMsgWhenSignalClientMatch) {
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);
    auto sigsel = std::initializer_list<uint32_t>{10};
    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*(clientMock.get()), getName()).WillByDefault(Return("TEST"));

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    ASSERT_EQ(server->receive(10000, sigsel, clientMock), message);
    ASSERT_NE(message->getClient(), nullptr);
    ASSERT_STREQ(message->getClient()->getName().c_str(), "TEST");
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_CallSocketReceive) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };
    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*socketMock, receive(_, _, 10000));
    handler->handleMessage(10000);
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_ReturnZeroWhenReceiveNotRegisteredMessage) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };

    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    auto message = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    ASSERT_EQ(handler->handleMessage(10000), 0);
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_ReturnErrorWhenNotReceiveMessage) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };

    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            return -1;
        }));

    ASSERT_EQ(handler->handleMessage(10000), -1);
}

TEST_F(LinxIpcSimpleServerTests, handleMessage_ReturnCallbackResult) {
    MockFunction<LinxIpcCallback> mockCallback;
    std::map<uint32_t, IpcContainer> handlers {
        {10, {mockCallback.AsStdFunction(), (void*)5}}
    };

    auto server = std::make_shared<LinxIpcSimpleServerImpl>(socketMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    auto message = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(mockCallback, Call(message.get(), (void*)5)).WillOnce(Return(5));
    ASSERT_EQ(handler->handleMessage(10000), 5);
}

MATCHER_P(MessageMatcher, reqId, "") {
    const LinxMessageIpc &message = arg;
    return message.getReqId() == reqId;
}

class LinxIpcExtendedServerTests : public testing::Test {
  public:
    NiceMock<LinxIpcQueueMock> *queueMock;
    NiceMock<LinxIpcSocketMock> *socketMock;

    void SetUp() {
        socketMock = new NiceMock<LinxIpcSocketMock>();
        queueMock = new NiceMock<LinxIpcQueueMock>();

        ON_CALL(*socketMock, receive(_, _, _)).WillByDefault(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));

        ON_CALL(*queueMock, get(_, _, _)).WillByDefault(Return(nullptr));
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
    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));

    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<LinxIpcClientPtr> fromOpt = std::make_optional(client);

    EXPECT_CALL(*queueMock, get(10000, SigselMatcher(sigsel), fromOpt));
    endpoint->receive(10000, sigsel, client);
}

TEST_F(LinxIpcExtendedServerTests, receive_callQueueWithNullOpt) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<LinxIpcClientPtr> fromOpt = std::nullopt;

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
        return msg;
    }));

    ASSERT_EQ(endpoint->receive(10000, sigsel), msg);
    ASSERT_NE(msg->getClient(), nullptr);
    ASSERT_STREQ(msg->getClient()->getName().c_str(), "TEST");
}

TEST_F(LinxIpcExtendedServerTests, start_DoNothingWhenAlreadyStarted) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    endpoint->start();
    endpoint->start();
}

TEST_F(LinxIpcExtendedServerTests, thread_DoNothingWhenStopped) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImplTask>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).Times(0);
    EXPECT_CALL(*queueMock, add(_)).Times(0);

    endpoint->stop();
    endpoint->task();
}

TEST_F(LinxIpcExtendedServerTests, thread_NotAddToQueueWhenReceiveReturnError) {
    auto endpoint = std::make_shared<LinxIpcExtendedServerImplTask>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(DoAll(
        Invoke([&endpoint](){endpoint->stop();}),
        Return(-1)));

    EXPECT_CALL(*queueMock, add(_)).Times(0);
    endpoint->task();
}

TEST_F(LinxIpcExtendedServerTests, thread_ReceiveHuntReqSendResponse) {

    auto endpoint = std::make_shared<LinxIpcExtendedServerImplTask>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            endpoint->stop();
            *msg = std::make_shared<LinxMessageIpc>(IPC_PING_REQ);
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*socketMock, send(MessageMatcher((uint32_t)IPC_PING_RSP), "TEST"));
    EXPECT_CALL(*queueMock, add(_)).Times(0);

    endpoint->task();
}

TEST_F(LinxIpcExtendedServerTests, thread_ReceiveMsgAddToQueue) {
    auto msg = std::make_shared<LinxMessageIpc>(10);

    auto endpoint = std::make_shared<LinxIpcExtendedServerImplTask>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint, &msg](LinxMessageIpcPtr *message, std::string *from, int timeout){
            endpoint->stop();
            *message = msg;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*queueMock, add(std::move(msg)));
    
    endpoint->task();
    testing::Mock::VerifyAndClearExpectations(queueMock);
}

TEST_F(LinxIpcExtendedServerTests, thread_ReceiveMsgAddToQueueError) {
    auto msg = std::make_shared<LinxMessageIpc>(10);

    auto endpoint = std::make_shared<LinxIpcExtendedServerImplTask>(socketMock, queueMock);
    endpoint->start();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&endpoint, &msg](LinxMessageIpcPtr *message, std::string *from, int timeout){
            endpoint->stop();
            *message = msg;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*queueMock, add(_)).WillOnce(Return(-1));
    endpoint->task();
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_CallQueueGet) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };

    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*queueMock, get(10000, _, _));
    handler->handleMessage(10000);
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_ReturnZeroWhenReceiveNotRegisteredMessage) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };

    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("TEST"));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(client);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return msg;
    }));

    ASSERT_EQ(handler->handleMessage(10000), 0);
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_ReturnErrorWhenNotReceiveMessage) {
    std::map<uint32_t, IpcContainer> handlers {
        {5, {[](LinxMessageIpc *msg, void *data) { return 0; }, nullptr}}
    };
    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([]() {
        return nullptr;
    }));

    ASSERT_EQ(handler->handleMessage(10000), -1);
}

TEST_F(LinxIpcExtendedServerTests, handleMessage_ReturnCallbackResult) {
    MockFunction<LinxIpcCallback> mockCallback;
    std::map<uint32_t, IpcContainer> handlers {
        {10, {mockCallback.AsStdFunction(), nullptr}}
    };

    auto server = std::make_shared<LinxIpcExtendedServerImpl>(socketMock, queueMock);
    auto handler = std::make_shared<LinxIpcHandlerImpl>(server, handlers);

    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("TEST"));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(client);

    EXPECT_CALL(*queueMock, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return msg;
    }));

    EXPECT_CALL(mockCallback, Call(msg.get(), nullptr)).WillOnce(Return(5));
    ASSERT_EQ(handler->handleMessage(10000), 5);
}
