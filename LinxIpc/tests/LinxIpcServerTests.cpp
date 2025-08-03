
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpcEndpointMock.h"
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcPrivate.h"

using namespace ::testing;

class LinxIpcServerImplTask: public LinxIpcServerImpl {
    public:
        LinxIpcServerImplTask(const LinxIpcEndpointPtr &endpoint, LinxQueue *queue)
            : LinxIpcServerImpl(endpoint, queue) {}

        void task() {
            LinxIpcServerImpl::task();
        }

        void start() {
            running = true;
        }

        void stop() {
            running = false;
        }
};

MATCHER_P(MessageMatcher, reqId, "") {
    const LinxMessageIpc &message = arg;
    return message.getReqId() == reqId;
}

class LinxIpcServerTests : public testing::Test {
  public:
    NiceMock<LinxIpcQueueMock> *queue;
    std::shared_ptr<NiceMock<LinxIpcEndpointMock>> endpoint = std::make_shared<NiceMock<LinxIpcEndpointMock>>();
    std::shared_ptr<NiceMock<LinxIpcClientMock>> clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();

    void SetUp() {
        queue = new NiceMock<LinxIpcQueueMock>();
        ON_CALL(*endpoint, getName()).WillByDefault(Return("ServerTest"));
        ON_CALL(*queue, get(_, _, _)).WillByDefault(Return(nullptr));
        ON_CALL(*queue, getFd()).WillByDefault(Return(1));

        ON_CALL(*(clientMock.get()), getName()).WillByDefault(Return("TEST"));
    }
};

TEST_F(LinxIpcServerTests, getPollFdReturnQueueGetFdResult) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpoint, queue);
    ASSERT_EQ(server->getPollFd(), 1);
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

TEST_F(LinxIpcServerTests, receive_callQueueWithClient) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpoint, queue);

    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<LinxIpcClientPtr> fromOpt = std::make_optional(clientMock);

    EXPECT_CALL(*queue, get(10000, SigselMatcher(sigsel), fromOpt));
    server->receive(10000, sigsel, clientMock);
}

TEST_F(LinxIpcServerTests, receive_callQueueWithNullOpt) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpoint, queue);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<LinxIpcClientPtr> fromOpt = std::nullopt;

    EXPECT_CALL(*queue, get(10000, SigselMatcher(sigsel), fromOpt));
    server->receive(10000, sigsel);
}

TEST_F(LinxIpcServerTests, receive_ReturnNullWhenQueueReturnNull) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpoint, queue);
    auto sigsel = std::initializer_list<uint32_t>{4};
    ASSERT_EQ(server->receive(10000, sigsel), nullptr);
}

TEST_F(LinxIpcServerTests, endpoint_receiveMsg) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpoint, queue);
    auto sigsel = std::initializer_list<uint32_t>{4};

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(clientMock);

    EXPECT_CALL(*queue, get(_, _, _)).WillOnce(Invoke([&msg]() {
        return msg;
    }));

    ASSERT_EQ(server->receive(10000, sigsel), msg);
    ASSERT_NE(msg->getClient(), nullptr);
    ASSERT_STREQ(msg->getClient()->getName().c_str(), "TEST");
}

TEST_F(LinxIpcServerTests, start_DoNothingWhenAlreadyStarted) {
    auto server = std::make_shared<LinxIpcServerImpl>(endpoint, queue);
    server->start();
    server->start();
}

TEST_F(LinxIpcServerTests, thread_DoNothingWhenStopped) {
    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);
    server->start();

    EXPECT_CALL(*endpoint, receive(_, _, _)).Times(0);
    EXPECT_CALL(*queue, add(_)).Times(0);

    server->stop();
    server->task();
}

TEST_F(LinxIpcServerTests, thread_NotAddToQueueWhenReceiveReturnError) {
    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);
    server->start();

    EXPECT_CALL(*endpoint, receive(_, _, _)).WillOnce(DoAll(
        Invoke([&server](){server->stop();}),
        Return(nullptr)));

    EXPECT_CALL(*queue, add(_)).Times(0);
    server->task();
}

TEST_F(LinxIpcServerTests, thread_ReceiveHuntReqSendResponse) {

    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);
    server->start();

    EXPECT_CALL(*endpoint, receive(_, _, _)).WillOnce(
        Invoke([&server, this](int timeoutMs, const std::vector<uint32_t> &sigsel, const std::optional<std::string> &client) {
            server->stop();
            auto msg = std::make_shared<LinxMessageIpc>(IPC_PING_REQ);
            msg->setClient(clientMock);
            return msg;
        }));

    EXPECT_CALL(*endpoint, send(MessageMatcher(IPC_PING_RSP), "TEST"));
    EXPECT_CALL(*queue, add(_)).Times(0);

    server->task();
}

TEST_F(LinxIpcServerTests, thread_ReceiveMsgAddToQueue) {

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(clientMock);

    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);
    server->start();

    EXPECT_CALL(*endpoint, receive(_, _, _)).WillOnce(
        Invoke([&server, &msg](int timeoutMs, const std::vector<uint32_t> &sigsel, const std::optional<std::string> &client) {
            server->stop();
            return msg;
        }));

    EXPECT_CALL(*queue, add(std::move(msg)));
    
    server->task();
    testing::Mock::VerifyAndClearExpectations(queue);
}

TEST_F(LinxIpcServerTests, thread_ReceiveMsgAddToQueueError) {

    auto msg = std::make_shared<LinxMessageIpc>(10);
    msg->setClient(clientMock);

    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);
    server->start();

    EXPECT_CALL(*endpoint, receive(_, _, _)).WillOnce(
        Invoke([&server, &msg](int timeoutMs, const std::vector<uint32_t> &sigsel, const std::optional<std::string> &client) {
            server->stop();
            return msg;
        }));

    EXPECT_CALL(*queue, add(_)).WillOnce(Return(-1));
    server->task();
}

TEST_F(LinxIpcServerTests, serverCreateCLient) {
    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);
    auto client = server->createClient("TestService");

    ASSERT_NE(client, nullptr);
    ASSERT_STREQ(client->getName().c_str(), "TestService");
}

TEST_F(LinxIpcServerTests, sendToNullReturnError) {
    
    auto msg = LinxMessageIpc(10);
    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);

    ASSERT_EQ(server->send(msg, nullptr), -1);
}

TEST_F(LinxIpcServerTests, sendCallEndpointSend) {
    
    auto msg = LinxMessageIpc(10);
    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);

    EXPECT_CALL(*endpoint, send(MessageMatcher(10U), "TEST"));
    server->send(msg, clientMock);
}

TEST_F(LinxIpcServerTests, sendReturnEndpointResult) {

    auto msg = LinxMessageIpc(10);
    auto server = std::make_shared<LinxIpcServerImplTask>(endpoint, queue);

    EXPECT_CALL(*endpoint, send(_, _)).WillOnce(Return(42));
    int ret = server->send(msg, clientMock);

    ASSERT_EQ(ret, 42);
}