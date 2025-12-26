#include "gtest/gtest.h"
#include "AfUnix.h"
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
        ON_CALL(*socketPtr, receive(_, _, _)).WillByDefault(Return(-1));

        queue = std::make_unique<NiceMock<LinxQueueMock>>();
        queuePtr = queue.get();
        ON_CALL(*queuePtr, get(_, _, _)).WillByDefault(Return(ByMove(LinxReceivedMessagePtr(nullptr))));
        ON_CALL(*queuePtr, getFd()).WillByDefault(Return(1));
    }
};

TEST_F(AfUnixServerTests, getPollFdReturnQueueGetFdResult) {
    auto server = AfUnixServer("TEST", socket, std::move(queue));
    ASSERT_EQ(server.getPollFd(), 1);
}

TEST_F(AfUnixServerTests, receive_callQueueWithClient) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));

    auto sigsel = std::initializer_list<uint32_t>{4};
    const IIdentifier *fromOpt = nullptr; // TODO: Update test to use identifier

    EXPECT_CALL(*queuePtr, get(10000, SigselMatcher(sigsel), fromOpt));
    server->receive(10000, sigsel, nullptr);
}

TEST_F(AfUnixServerTests, receive_callQueueWithNullOpt) {
    auto server = AfUnixServer("TEST", socket, std::move(queue));
    auto sigsel = std::initializer_list<uint32_t>{4};
    const IIdentifier *fromOpt = nullptr;

    EXPECT_CALL(*queuePtr, get(10000, SigselMatcher(sigsel), fromOpt));
    server.receive(10000, sigsel);
}

TEST_F(AfUnixServerTests, receive_ReturnNullWhenQueueReturnNull) {
    auto server = AfUnixServer("TEST", socket, std::move(queue));
    auto sigsel = std::initializer_list<uint32_t>{4};

    ASSERT_EQ(server.receive(10000, sigsel), nullptr);
}

TEST_F(AfUnixServerTests, endpoint_receiveMsg) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));
    auto sigsel = std::initializer_list<uint32_t>{4};

    auto msg = std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
        .message = std::make_unique<RawMessage>(10),
        .from = std::make_unique<StringIdentifier>("TEST")
    });

    EXPECT_CALL(*queuePtr, get(_, _, _)).WillOnce(Invoke(
        [msg = std::move(msg)](int, const std::vector<uint32_t>&, const IIdentifier*) mutable {
            return std::move(msg);
        }
    ));

    auto result = server->receive(10000, sigsel);
    ASSERT_NE(result, nullptr);
    ASSERT_TRUE(result->from->isEqual(StringIdentifier("TEST")));
    ASSERT_EQ(result->message->getReqId(), 10U);
}

TEST_F(AfUnixServerTests, stop_DoNothingWhenNotStarted) {
    auto server = AfUnixServer("TEST", socket, std::move(queue));
    server.stop();
}

TEST_F(AfUnixServerTests, stop_StopsTheWorkerThread) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));
    server->start();
    server->stop();
    server->start();
    server->stop();
}

TEST_F(AfUnixServerTests, destructor_CallsStopAndClearsQueue) {
    EXPECT_CALL(*queuePtr, clear()).Times(1);
    {
        auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));
        server->start();
    }
    // Destructor should have called stop() and queue->clear()
}

// createContext test removed - method no longer exists after endpoint removal

TEST_F(AfUnixServerTests, task_ReceivesMessageAndAddsToQueue) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));

    // Simulate socket receiving a message
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .WillOnce(Invoke([](RawMessagePtr *msg, StringIdentifier *from, int) {
            *msg = std::make_unique<RawMessage>(42);
            *from = StringIdentifier("CLIENT1");
            return 4;
        }))
        .WillRepeatedly(testing::Return(-1)); // Timeout for subsequent calls

    EXPECT_CALL(*queuePtr, add(_)).WillOnce(testing::Return(0));

    auto ret = server->start();
    ASSERT_TRUE(ret);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    server->stop();
}

TEST_F(AfUnixServerTests, task_HandlesPingRequest) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));

    // Simulate socket receiving a ping request
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .WillOnce(Invoke([](RawMessagePtr *msg, StringIdentifier *from, int) {
            *msg = std::make_unique<RawMessage>(IPC_PING_REQ);
            *from = StringIdentifier("CLIENT1");
            return 4;
        }))
        .WillRepeatedly(testing::Return(-1)); // Timeout for subsequent calls

    // Expect ping response to be sent
    EXPECT_CALL(*socketPtr, send(testing::_, StringIdentifier("CLIENT1"))).Times(1);

    // Ping messages should NOT be added to queue
    EXPECT_CALL(*queuePtr, add(_)).Times(0);

    auto ret = server->start();
    ASSERT_TRUE(ret);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    server->stop();
}

TEST_F(AfUnixServerTests, task_DiscardsMessageWhenQueueFull) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));

    // Simulate socket receiving a message
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .WillOnce(Invoke([](RawMessagePtr *msg, StringIdentifier *from, int) {
            *msg = std::make_unique<RawMessage>(42);
            *from = StringIdentifier("CLIENT1");
            return 4;
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

TEST_F(AfUnixServerTests, task_HandlesSocketClose) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));

    // Simulate socket timing out
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .Times(1)
        .WillRepeatedly(testing::Return(0));

    // No messages should be added to queue
    EXPECT_CALL(*queuePtr, add(_)).Times(0);

    auto ret = server->start();
    ASSERT_TRUE(ret);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    server->stop();
}

TEST_F(AfUnixServerTests, receive_WithFromContext_PassesCorrectOptional) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));

    auto sigsel = std::initializer_list<uint32_t>{10, 20};

    EXPECT_CALL(*queuePtr, get(5000, SigselMatcher(sigsel), nullptr));
    server->receive(5000, sigsel, LINX_ANY_FROM);
}

TEST_F(AfUnixServerTests, receive_ReturnsMessageWithCorrectReqId) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));
    auto sigsel = std::initializer_list<uint32_t>{100};

    auto msg = std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
        .message = std::make_unique<RawMessage>(100),
        .from = std::make_unique<StringIdentifier>("SENDER")
    });

    EXPECT_CALL(*queuePtr, get(_, _, _)).WillOnce(Invoke(
        [msg = std::move(msg)](int, const std::vector<uint32_t>&, const IIdentifier*) mutable {
            return std::move(msg);
        }
    ));

    auto result = server->receive(1000, sigsel);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->message->getReqId(), 100U);
}

TEST_F(AfUnixServerTests, receive_MultipleSignalsInSigsel) {
    auto server = AfUnixServer("TEST", socket, std::move(queue));

    auto sigsel = std::initializer_list<uint32_t>{1, 2, 3, 4, 5};
    const IIdentifier *fromOpt = nullptr;

    EXPECT_CALL(*queuePtr, get(1000, SigselMatcher(sigsel), fromOpt));
    server.receive(1000, sigsel);
}

TEST_F(AfUnixServerTests, stop_MultipleCalls_SafeToCall) {
    auto server = std::make_shared<AfUnixServer>("TEST", socket, std::move(queue));
    server->start();
    server->stop();
    server->stop(); // Should be safe to call multiple times
    server->stop();
}
