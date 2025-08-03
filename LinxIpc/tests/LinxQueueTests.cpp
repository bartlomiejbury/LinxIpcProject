
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcEventFdMock.h"
#include "LinxIpcClientMock.h"
#include "PthreadMock.h"
#include "LinxIpc.h"
#include "LinxQueueImpl.h"

using namespace ::testing;

class LinxQueueTests : public testing::Test {
   public:
    NiceMock<LinxIpcEventFdMock> *efdMock;

    void SetUp() {
        efdMock = new NiceMock<LinxIpcEventFdMock>();
        ON_CALL(*efdMock, getFd()).WillByDefault(Return(1));
    }

    LinxMessageIpcPtr createMsgFromClient(const std::string &clientName, uint32_t reqId) {

        auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
        ON_CALL(*client, getName()).WillByDefault(Return(clientName));

        LinxMessageIpcPtr msg = std::make_shared<LinxMessageIpc>(reqId);
        msg->setClient(client);
        return msg;
    }
};

TEST_F(LinxQueueTests, addToQueue_ReturnErrorWhenMaximumSizeReached) {
    auto queue = LinxQueueImpl(efdMock, 1);
    LinxMessageIpcPtr msg = createMsgFromClient("from", 1);

    ASSERT_EQ(queue.add(msg), 0);
    ASSERT_EQ(queue.add(msg), -1);
}

TEST_F(LinxQueueTests, clearQueue_DecrementSize) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg = createMsgFromClient("from", 1);

    queue.add(msg);
    queue.add(msg);

    ASSERT_EQ(queue.size(), 2);

    queue.clear();
    ASSERT_EQ(queue.size(), 0);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
    queue.add(msg2);

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_NotDecretementSizeWHenELementNotFound) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
    queue.add(msg2);

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt);
    ASSERT_EQ(queue.size(), 2);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
    queue.add(msg2);

    auto msg = queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Immediate_DecrementSizeWhenElementFound) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
    queue.add(msg2);

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client3 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client3, getName()).WillByDefault(Return("from3"));

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
    queue.add(msg2);

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client3)), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    auto msg = queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_CallWaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    NiceMock<PthreadMock> pthreadMock;
    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
            queue.add(msg2);
            return 0;
        });

    auto msg = queue.get(INFINITE_TIMEOUT, {2, 3}, std::nullopt);
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMessageWhenSignalNrArriveInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    NiceMock<PthreadMock> pthreadMock;
    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg2 = createMsgFromClient("from3", 4);
            queue.add(msg2);
            return 0;
        })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
            queue.add(msg2);
            return 0;
        });

    auto msg = queue.get(INFINITE_TIMEOUT, {2, 3}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
    queue.add(msg2);

    auto msg = queue.get(INFINITE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_CallWaitWhenNoSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    NiceMock<PthreadMock> pthreadMock;
    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, &client2]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client2);
            queue.add(msg2);
            return 0;
        });

    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMessageSignalSenderArriveInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    NiceMock<PthreadMock> pthreadMock;
    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg2 = createMsgFromClient("from3", 2);
            queue.add(msg2);
            return 0;
        })
        .WillOnce([&queue, &client2]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client2);
            queue.add(msg2);
            return 0;
        });

    auto msg = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    auto msg = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_DecrementSizeWhenSignalArrive) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    NiceMock<PthreadMock> pthreadMock;
    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg = createMsgFromClient("from3", 2);
            queue.add(msg);
            return 0;
        })
        .WillOnce([&queue, &client2]() {
            LinxMessageIpcPtr msg = std::make_shared<LinxMessageIpc>(2);
            msg->setClient(client2);
            queue.add(msg);
            return 0;
        });

    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Infinite_DecrementSizeWhenElementInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    ASSERT_EQ(queue.size(), 2);
    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Timeout_CallWaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    NiceMock<PthreadMock> pthreadMock;
    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg = createMsgFromClient("from2", 2);
            queue.add(msg);
            return 0;
        });

    auto msg = queue.get(500, {2, 3}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Timeout_GetCorrectMessageSignalInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    NiceMock<PthreadMock> pthreadMock;
    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg = createMsgFromClient("from3", 4);
            queue.add(msg);
            return 0;
        })
        .WillOnce([&queue, this]() {
            LinxMessageIpcPtr msg = createMsgFromClient("from2", 2);
            queue.add(msg);
            return 0;
        });

    auto msg = queue.get(500, {2, 3}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Timeout_ReturnNullWhenWaitTimedOut) {
    auto queue = LinxQueueImpl(efdMock, 2);
    ASSERT_EQ(queue.get(500, {2, 3}, std::nullopt), nullptr);
}

TEST_F(LinxQueueTests, get_Timeout_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = createMsgFromClient("from1", 1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = createMsgFromClient("from2", 2);
    queue.add(msg2);

    auto msg = queue.get(500, {3, 2}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, getFdReturnefdFd) {
    auto queue = LinxQueueImpl(efdMock, 2);
    ASSERT_EQ(queue.getFd(), 1);
}

TEST_F(LinxQueueTests, testGetDuration) {

    static constexpr int time = 1000;
    static constexpr int margin = 10;

    LinxQueueImpl queue = LinxQueueImpl(efdMock, 10);

    auto startTime = std::chrono::steady_clock::now();
    queue.get(time, {10}, std::nullopt);
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    ASSERT_GE(duration.count(), time-margin) << "Get should take at least " << time-margin <<" ms";
    ASSERT_LE(duration.count(), time+margin) << "Get should not take more than " << time+margin <<" ms";
}