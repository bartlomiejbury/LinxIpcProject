
#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "PthreadMock.h"
#include "LinxIpcEventFdMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpc.h"
#include "LinxQueueImpl.h"

using namespace ::testing;

class LinxQueueTests : public testing::Test {
   public:
    NiceMock<SystemMock> systemMock;
    NiceMock<PthreadMock> pthreadMock;
    NiceMock<LinxIpcEventFdMock> *efdMock;
    struct timespec currentTime = {};

    void SetUp() {

        efdMock = new NiceMock<LinxIpcEventFdMock>();

        ON_CALL(systemMock, clock_gettime(_, _))
            .WillByDefault(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));

        ON_CALL(pthreadMock, pthread_mutex_init(_, _)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_mutex_destroy(_)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_mutex_lock(_)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_mutex_unlock(_)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_condattr_init(_)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_cond_init(_, _)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_cond_destroy(_)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_cond_timedwait(_, _, _)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_cond_wait(_, _)).WillByDefault(Return(0));
        ON_CALL(pthreadMock, pthread_cond_broadcast(_)).WillByDefault(Return(0));
    }
};

TEST_F(LinxQueueTests, addToQueue_ReturnErrorWhenMaximumSizeReached) {
    auto queue = LinxQueueImpl(efdMock, 1);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("from"));

    LinxMessageIpcPtr msg = std::make_shared<LinxMessageIpc>(1);
    msg->setClient(client);

    ASSERT_EQ(queue.add(msg), 0);
    ASSERT_EQ(queue.add(msg), -1);
}

TEST_F(LinxQueueTests, clearQueue_DecrementSize) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("from"));

    LinxMessageIpcPtr msg = std::make_shared<LinxMessageIpc>(1);
    msg->setClient(client);

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

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_NotDecretementSizeWHenELementNotFound) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client2);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt);
    ASSERT_EQ(queue.size(), 2);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    auto msg = queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Immediate_DecrementSizeWhenElementFound) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));
    auto client3 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client3, getName()).WillByDefault(Return("from3"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client3)), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
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
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client, getName()).WillByDefault(Return("from2"));

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, &client]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client);
            queue.add(msg2);
            return 0;
        });

    auto msg = queue.get(INFINITE_TIMEOUT, {2, 3}, std::nullopt);
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMessageWhenSignalNrArriveInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from3"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, &client1]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(4);
            msg2->setClient(client1);
            queue.add(msg2);
            return 0;
        })
        .WillOnce([&queue, &client2]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client2);
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
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
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
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from3"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, &client1]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client1);
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
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
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
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from3"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, &client1]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client1);
            queue.add(msg2);
            return 0;
        })
        .WillOnce([&queue, &client2]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client2);
            queue.add(msg2);
            return 0;
        });

    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Infinite_DecrementSizeWhenElementInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    ASSERT_EQ(queue.size(), 2);
    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional(client2));
    ASSERT_EQ(queue.size(), 1);
}

MATCHER_P2(TimespecMatcher, sec, nsec, "") {
    struct timespec *currentTime = (struct timespec *)arg;
    return currentTime->tv_sec == sec && currentTime->tv_nsec == nsec;
}

TEST_F(LinxQueueTests, get_Timeout_CallWaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from2"));

    currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, TimespecMatcher(2, 200000000)))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, &client1]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client1);
            queue.add(msg2);
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
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from3"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue, &client1]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(4);
            msg2->setClient(client1);
            queue.add(msg2);
            return 0;
        })
        .WillOnce([&queue, &client2]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            msg2->setClient(client2);
            queue.add(msg2);
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

    currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, _)).WillOnce([&queue]() {
        return ETIMEDOUT;
    });

    ASSERT_EQ(queue.get(500, {2, 3}, std::nullopt), nullptr);
}

TEST_F(LinxQueueTests, get_Timeout_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);
    auto client1 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client1, getName()).WillByDefault(Return("from1"));
    auto client2 = std::make_shared<NiceMock<LinxIpcClientMock>>();
    ON_CALL(*client2, getName()).WillByDefault(Return("from2"));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    msg1->setClient(client1);
    queue.add(msg1);

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    msg2->setClient(client2);
    queue.add(msg2);

    auto msg = queue.get(500, {3, 2}, std::nullopt);
    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);

    auto client = msg->getClient()->getName();
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, getFdReturnefdFd) {
    auto queue = LinxQueueImpl(efdMock, 2);

    EXPECT_CALL(*efdMock, getFd()).WillOnce(Return(1));
    ASSERT_EQ(queue.getFd(), 1);
}
