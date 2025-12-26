#include <thread>
#include "gtest/gtest.h"
#include "AfUnix.h"
#include "LinxClientMock.h"
#include "LinxEventFdMock.h"
#include "LinxIpc.h"
#include "LinxQueue.h"

using namespace ::testing;

class LinxQueueTests : public testing::Test {
   public:
    std::unique_ptr<LinxEventFdMock> efdMock;
    LinxEventFdMock* efdPtr;

    void SetUp() {
        efdMock = std::make_unique<NiceMock<LinxEventFdMock>>();
        efdPtr = efdMock.get();
        ON_CALL(*efdPtr, getFd()).WillByDefault(Return(1));
    }

    LinxReceivedMessagePtr createMsgFromClient(const std::string &clientName, uint32_t reqId) {
        return std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
            .message = std::make_unique<RawMessage>(reqId),
            .from = std::make_unique<StringIdentifier>(clientName),
        });
    }
};

TEST_F(LinxQueueTests, addToQueue_ReturnErrorWhenMaximumSizeReached) {
    auto queue = LinxQueue(std::move(efdMock), 1);
    LinxReceivedMessagePtr msg1 = createMsgFromClient("from", 1);
    LinxReceivedMessagePtr msg2 = createMsgFromClient("from", 1);
    ASSERT_EQ(queue.add(std::move(msg1)), 0);
    ASSERT_EQ(queue.add(std::move(msg2)), -1);
}

TEST_F(LinxQueueTests, clearQueue_DecrementSize) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from", 1);
    LinxReceivedMessagePtr msg2 = createMsgFromClient("from", 1);

    queue.add(std::move(msg1));
    queue.add(std::move(msg2));

    ASSERT_EQ(queue.size(), 2);

    queue.clear();
    ASSERT_EQ(queue.size(), 0);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalNrInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    queue.add(std::move(msg2));

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, {3, 4}, nullptr), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_NotDecretementSizeWHenElementNotFound) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    queue.add(std::move(msg2));

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 4}, nullptr);
    ASSERT_EQ(queue.size(), 2);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    auto msg2Ptr = msg2->message.get();
    StringIdentifier from2("from2");
    queue.add(std::move(msg2));

    auto msg = queue.get(IMMEDIATE_TIMEOUT, {3, 2}, nullptr);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message.get(), msg2Ptr);
    ASSERT_TRUE(msg->from->isEqual(from2));
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Immediate_DecrementSizeWhenElementFound) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    queue.add(std::move(msg2));

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 2}, nullptr);
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalSenderInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);
    StringIdentifier from3Identifier("from3");

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    queue.add(std::move(msg2));

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, &from3Identifier), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);
    StringIdentifier from2Identifier("from2");

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    auto msg2Ptr = msg2->message.get();
    StringIdentifier from2("from2");
    queue.add(std::move(msg2));

    auto msg = queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, &from2Identifier);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message.get(), msg2Ptr);
    ASSERT_TRUE(msg->from->isEqual(from2));
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Infinite_CallWaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    std::thread producer([&queue, this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
        queue.add(std::move(msg1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
        queue.add(std::move(msg2));
    });

    auto msg = queue.get(INFINITE_TIMEOUT, {2, 3}, nullptr);
    producer.join();
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMessageWhenSignalNrArriveInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 5);

    std::thread producer([&queue, this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
        queue.add(std::move(msg1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg2 = createMsgFromClient("from3", 4);
        queue.add(std::move(msg2));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg3 = createMsgFromClient("from2", 2);
        queue.add(std::move(msg3));
    });

    auto msg = queue.get(INFINITE_TIMEOUT, {2, 3}, nullptr);
    producer.join();
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    auto msg2Ptr = msg2->message.get();
    StringIdentifier from2("from2");
    queue.add(std::move(msg2));

    auto msg = queue.get(INFINITE_TIMEOUT, {3, 2}, nullptr);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message.get(), msg2Ptr);
    ASSERT_TRUE(msg->from->isEqual(from2));
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Infinite_CallWaitWhenNoSignalSenderInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);
    StringIdentifier from2Identifier("from2");

    std::thread producer([&queue, this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
        queue.add(std::move(msg1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
        queue.add(std::move(msg2));
    });

    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, &from2Identifier);
    producer.join();
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMessageSignalSenderArriveInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 5);
    StringIdentifier from2Identifier("from2");

    std::thread producer([&queue, this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
        queue.add(std::move(msg1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg2 = createMsgFromClient("from3", 2);
        queue.add(std::move(msg2));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg3 = createMsgFromClient("from2", 2);
        queue.add(std::move(msg3));
    });

    auto msg = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, &from2Identifier);
    producer.join();
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);
    StringIdentifier from2Identifier("from2");

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    auto msg2Ptr = msg2->message.get();
    StringIdentifier from2("from2");
    queue.add(std::move(msg2));

    auto msg = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, &from2Identifier);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message.get(), msg2Ptr);
    ASSERT_TRUE(msg->from->isEqual(from2));
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Infinite_DecrementSizeWhenSignalArrive) {
    auto queue = LinxQueue(std::move(efdMock), 5);
    StringIdentifier from2Identifier("from2");

    std::thread producer([&queue, this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
        queue.add(std::move(msg1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg2 = createMsgFromClient("from3", 2);
        queue.add(std::move(msg2));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg3 = createMsgFromClient("from2", 2);
        queue.add(std::move(msg3));
    });

    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, &from2Identifier);
    producer.join();
    ASSERT_EQ(queue.size(), 2);
}

TEST_F(LinxQueueTests, get_Infinite_DecrementSizeWhenElementInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);
    StringIdentifier from2Identifier("from2");

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    queue.add(std::move(msg2));

    ASSERT_EQ(queue.size(), 2);
    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, &from2Identifier);
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Timeout_CallWaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    std::thread producer([&queue, this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
        queue.add(std::move(msg1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
        queue.add(std::move(msg2));
    });

    auto msg = queue.get(500, {2, 3}, nullptr);
    producer.join();
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Timeout_GetCorrectMessageSignalInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 5);

    std::thread producer([&queue, this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
        queue.add(std::move(msg1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg2 = createMsgFromClient("from3", 4);
        queue.add(std::move(msg2));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LinxReceivedMessagePtr msg3 = createMsgFromClient("from2", 2);
        queue.add(std::move(msg3));
    });

    auto msg = queue.get(500, {2, 3}, nullptr);
    producer.join();
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, get_Timeout_ReturnNullWhenWaitTimedOut) {
    auto queue = LinxQueue(std::move(efdMock), 2);
    ASSERT_EQ(queue.get(500, {2, 3}, nullptr), nullptr);
}

TEST_F(LinxQueueTests, get_Timeout_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueue(std::move(efdMock), 2);

    LinxReceivedMessagePtr msg1 = createMsgFromClient("from1", 1);
    queue.add(std::move(msg1));

    LinxReceivedMessagePtr msg2 = createMsgFromClient("from2", 2);
    queue.add(std::move(msg2));
    auto msg = queue.get(500, {3, 2}, nullptr);

    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->message->getReqId(), 2);
}

TEST_F(LinxQueueTests, getFdReturnefdFd) {
    auto queue = LinxQueue(std::move(efdMock), 2);
    ASSERT_EQ(queue.getFd(), 1);
}

TEST_F(LinxQueueTests, testGetDuration) {

    static constexpr int time = 1000;
    static constexpr int margin = 10;

    auto queue = LinxQueue(std::move(efdMock), 2);

    auto startTime = std::chrono::steady_clock::now();
    queue.get(time, {10}, nullptr);
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    ASSERT_GE(duration.count(), time-margin) << "Get should take at least " << time-margin <<" ms";
    ASSERT_LE(duration.count(), time+margin) << "Get should not take more than " << time+margin <<" ms";
}
