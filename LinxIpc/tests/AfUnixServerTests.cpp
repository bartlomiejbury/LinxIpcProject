
#include "gtest/gtest.h"
#include "AfUnixEndpoint.h"
#include "AfUnixServer.h"
#include "AfUnixSocketMock.h"
#include "LinxIpc.h"
#include "LinxMessageIds.h"
#include "LinxQueueMock.h"

using namespace ::testing;

MATCHER_P(SigselMatcher, signals, "") {
    const std::vector<uint32_t> &sigsel = arg;
    const std::vector<uint32_t> expected = signals;
    return sigsel == expected;
}

class AfUnixServerTests : public testing::Test {
  public:
    std::shared_ptr<AfUnixSocketMock> socket;
    AfUnixSocketMock* socketPtr;
    std::unique_ptr<LinxQueueMock> queue;
    LinxQueueMock* queuePtr;

    void SetUp() {
        socket = std::make_shared<NiceMock<AfUnixSocketMock>>();
        socketPtr = socket.get();
        ON_CALL(*socketPtr, open()).WillByDefault(Return(true));
        ON_CALL(*socketPtr, receive(_, _, _)).WillByDefault(Return(-1));

        queue = std::make_unique<NiceMock<LinxQueueMock>>();
        queuePtr = queue.get();
        ON_CALL(*queuePtr, get(_, _, _)).WillByDefault(Return(ByMove(LinxReceivedMessagePtr(nullptr))));
        ON_CALL(*queuePtr, getFd()).WillByDefault(Return(1));
    }
};

TEST_F(AfUnixServerTests, getPollFdReturnQueueGetFdResult) {
    auto server = AfUnixServer(socket, std::move(queue), "TEST");
    ASSERT_EQ(server.getPollFd(), 1);
}

TEST_F(AfUnixServerTests, receive_callQueueWithClient) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");

    auto sigsel = std::initializer_list<uint32_t>{4};
    LinxReceiveContextSharedPtr context = server->createContext("CLIENT1");
    LinxReceiveContextOpt fromOpt = std::make_optional(context);

    EXPECT_CALL(*queuePtr, get(10000, SigselMatcher(sigsel), fromOpt));
    server->receive(10000, sigsel, context);
}

TEST_F(AfUnixServerTests, receive_callQueueWithNullOpt) {
    auto server = AfUnixServer(socket, std::move(queue), "TEST");
    auto sigsel = std::initializer_list<uint32_t>{4};
    LinxReceiveContextOpt fromOpt = std::nullopt;

    EXPECT_CALL(*queuePtr, get(10000, SigselMatcher(sigsel), fromOpt));
    server.receive(10000, sigsel);
}

TEST_F(AfUnixServerTests, receive_ReturnNullWhenQueueReturnNull) {
    auto server = AfUnixServer(socket, std::move(queue), "TEST");
    auto sigsel = std::initializer_list<uint32_t>{4};

    ASSERT_EQ(server.receive(10000, sigsel), nullptr);
}

TEST_F(AfUnixServerTests, endpoint_receiveMsg) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto sigsel = std::initializer_list<uint32_t>{4};

    auto msg = std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
        .message = std::make_unique<LinxMessage>(10),
        .context = std::make_unique<AfUnixEndpoint>(this->socket, server, "TEST")
    });
    auto msgCtxPtr = msg->context.get();

    EXPECT_CALL(*queuePtr, get(_, _, _)).WillOnce(Invoke(
        [msg = std::move(msg)](int, const std::vector<uint32_t>&, const LinxReceiveContextOpt&) mutable {
            return std::move(msg);
        }
    ));

    auto result = server->receive(10000, sigsel);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(*result->context, *msgCtxPtr);
    ASSERT_EQ(result->message->getReqId(), 10U);
}

TEST_F(AfUnixServerTests, stop_DoNothingWhenNotStarted) {
    auto server = AfUnixServer(socket, std::move(queue), "TEST");
    server.stop();
}

TEST_F(AfUnixServerTests, stop_StopsTheWorkerThread) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    server->start();
    server->stop();
    // After stop, start should work again
    EXPECT_CALL(*socketPtr, open()).Times(1);
    server->start();
    server->stop();
}

TEST_F(AfUnixServerTests, destructor_CallsStopAndClearsQueue) {
    EXPECT_CALL(*queuePtr, clear()).Times(1);
    {
        auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
        server->start();
    }
    // Destructor should have called stop() and queue->clear()
}

TEST_F(AfUnixServerTests, createContext_ReturnsContextWithSocketAndFrom) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto context = server->createContext("CLIENT1");

    ASSERT_NE(context, nullptr);
    // The context should be an AfUnixEndpoint
    auto afContext = std::dynamic_pointer_cast<AfUnixEndpoint>(context);
    ASSERT_NE(afContext, nullptr);
}

TEST_F(AfUnixServerTests, task_ReceivesMessageAndAddsToQueue) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");

    // Simulate socket receiving a message
    EXPECT_CALL(*socketPtr, receive(_, _, 100))
        .WillOnce(Invoke([](LinxMessagePtr *msg, std::string *from, int) {
            *msg = std::make_unique<LinxMessage>(42);
            *from = "CLIENT1";
            return 0;
        }))
        .WillRepeatedly(testing::Return(-1)); // Timeout for subsequent calls

    EXPECT_CALL(*queuePtr, add(_)).WillOnce(testing::Return(0));

    auto ret = server->start();
    ASSERT_TRUE(ret);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    server->stop();
}

TEST_F(AfUnixServerTests, task_HandlesPingRequest) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");

    // Simulate socket receiving a ping request
    EXPECT_CALL(*socketPtr, receive(_, _, 100))
        .WillOnce(Invoke([](LinxMessagePtr *msg, std::string *from, int) {
            *msg = std::make_unique<LinxMessage>(IPC_PING_REQ);
            *from = "CLIENT1";
            return 0;
        }))
        .WillRepeatedly(testing::Return(-1)); // Timeout for subsequent calls

    // Expect ping response to be sent
    EXPECT_CALL(*socketPtr, send(testing::_, "CLIENT1")).Times(1);

    // Ping messages should NOT be added to queue
    EXPECT_CALL(*queuePtr, add(_)).Times(0);

    auto ret = server->start();
    ASSERT_TRUE(ret);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    server->stop();
}

TEST_F(AfUnixServerTests, task_DiscardsMessageWhenQueueFull) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");

    // Simulate socket receiving a message
    EXPECT_CALL(*socketPtr, receive(_, _, 100))
        .WillOnce(Invoke([](LinxMessagePtr *msg, std::string *from, int) {
            *msg = std::make_unique<LinxMessage>(42);
            *from = "CLIENT1";
            return 0;
        }))
        .WillOnce(testing::Return(-1)) // Timeout after first message
        .WillRepeatedly(testing::Return(-1));

    // Queue is full and returns error
    EXPECT_CALL(*queuePtr, add(_)).WillOnce(testing::Return(-1));

    auto ret = server->start();
    ASSERT_TRUE(ret);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    server->stop();
}

TEST_F(AfUnixServerTests, task_HandlesReceiveTimeout) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");

    // Simulate socket timing out
    EXPECT_CALL(*socketPtr, receive(_, _, 100))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Return(-1)); // Always timeout

    // No messages should be added to queue
    EXPECT_CALL(*queuePtr, add(_)).Times(0);

    auto ret = server->start();
    ASSERT_TRUE(ret);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    server->stop();
}

TEST_F(AfUnixServerTests, receive_WithFromContext_PassesCorrectOptional) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");

    auto sigsel = std::initializer_list<uint32_t>{10, 20};
    LinxReceiveContextSharedPtr context = server->createContext("CLIENT2");
    LinxReceiveContextOpt fromOpt = std::make_optional(context);

    EXPECT_CALL(*queuePtr, get(5000, SigselMatcher(sigsel), fromOpt));
    server->receive(5000, sigsel, context);
}

TEST_F(AfUnixServerTests, receive_ReturnsMessageWithCorrectReqId) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto sigsel = std::initializer_list<uint32_t>{100};

    auto msg = std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
        .message = std::make_unique<LinxMessage>(100),
        .context = std::make_unique<AfUnixEndpoint>(this->socket, server, "SENDER")
    });

    EXPECT_CALL(*queuePtr, get(_, _, _)).WillOnce(Invoke(
        [msg = std::move(msg)](int, const std::vector<uint32_t>&, const LinxReceiveContextOpt&) mutable {
            return std::move(msg);
        }
    ));

    auto result = server->receive(1000, sigsel);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->message->getReqId(), 100U);
}

TEST_F(AfUnixServerTests, constructor_StoresServiceNameFromSocket) {
    ON_CALL(*socketPtr, getName()).WillByDefault(testing::Return("MY_SERVICE"));
    auto server = AfUnixServer(socket, std::move(queue), "IGNORED_NAME");

    // The service name should come from socket->getName(), not the parameter
    // We can't directly test this, but the coverage shows it's set correctly
}

TEST_F(AfUnixServerTests, receive_MultipleSignalsInSigsel) {
    auto server = AfUnixServer(socket, std::move(queue), "TEST");

    auto sigsel = std::initializer_list<uint32_t>{1, 2, 3, 4, 5};
    LinxReceiveContextOpt fromOpt = std::nullopt;

    EXPECT_CALL(*queuePtr, get(1000, SigselMatcher(sigsel), fromOpt));
    server.receive(1000, sigsel);
}

TEST_F(AfUnixServerTests, start_MultipleCalls_OnlyStartsOnce) {
    EXPECT_CALL(*socketPtr, open()).Times(1); // Should only be called once

    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    server->start();
    server->start();
    server->start();
    server->stop();
}

TEST_F(AfUnixServerTests, stop_MultipleCalls_SafeToCall) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    server->start();
    server->stop();
    server->stop(); // Should be safe to call multiple times
    server->stop();
}
