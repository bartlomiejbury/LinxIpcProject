
#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "PthreadMock.h"
#include "LinxIpcQueueFdMock.h"
#include "LinxIpc.h"
#include "LinxQueueImpl.h"

using namespace ::testing;

class LinxQueueTests : public testing::Test {
   public:
    NiceMock<SystemMock> systemMock;
    NiceMock<PthreadMock> pthreadMock;
    NiceMock<LinxIpcQueueFdMock> *efdMock;
    struct timespec currentTime = {};

    void SetUp() {

        efdMock = new NiceMock<LinxIpcQueueFdMock>();

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
    LinxMessageIpcPtr msg = std::make_shared<LinxMessageIpc>(1);

    ASSERT_EQ(queue.add(msg, "from"), 0);
    ASSERT_EQ(queue.add(msg, "from"), -1);
}

TEST_F(LinxQueueTests, clearQueue_DecrementSize) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg, "from");
    queue.add(msg, "from");

    ASSERT_EQ(queue.size(), 2);

    queue.clear();
    ASSERT_EQ(queue.size(), 0);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_NotDecretementSizeWHenELementNotFound) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt);
    ASSERT_EQ(queue.size(), 2);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto container = queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_NE(msg, nullptr);
    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Immediate_DecrementSizeWhenElementFound) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(queue.size(), 2);

    queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from3")), nullptr);
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto container = queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_CallWaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    auto msg = queue.get(INFINITE_TIMEOUT, {2, 3}, std::nullopt);
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMessageWhenSignalNrArriveInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(4);
            queue.add(msg2, "from3");
            return 0;
        })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    auto container = queue.get(INFINITE_TIMEOUT, {2, 3}, std::nullopt);
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto container = queue.get(INFINITE_TIMEOUT, {3, 2}, std::nullopt);
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_CallWaitWhenNoSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMessageSignalSenderArriveInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from3");
            return 0;
        })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    auto container = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto container = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Infinite_DecrementSizeWhenSignalArrive) {
    auto queue = LinxQueueImpl(efdMock, 2);

    EXPECT_CALL(pthreadMock, pthread_cond_wait(_, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from3");
            return 0;
        })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    ASSERT_EQ(queue.size(), 1);
}

TEST_F(LinxQueueTests, get_Infinite_DecrementSizeWhenElementInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(queue.size(), 2);
    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    ASSERT_EQ(queue.size(), 1);
}

MATCHER_P2(TimespecMatcher, sec, nsec, "") {
    struct timespec *currentTime = (struct timespec *)arg;
    return currentTime->tv_sec == sec && currentTime->tv_nsec == nsec;
}

TEST_F(LinxQueueTests, get_Timeout_CallWaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, TimespecMatcher(2, 200000000)))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    auto container = queue.get(500, {2, 3}, std::nullopt);
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, get_Timeout_GetCorrectMessageSignalInQueue) {
    auto queue = LinxQueueImpl(efdMock, 2);

    currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, _))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(4);
            queue.add(msg2, "from3");
            return 0;
        })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    auto container = queue.get(500, {2, 3}, std::nullopt);
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
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

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto container = queue.get(500, {3, 2}, std::nullopt);
    auto msg = std::get<LinxMessageIpcPtr>(*container);
    auto client = std::get<std::string>(*container);

    ASSERT_EQ(msg->getReqId(), 2);
    ASSERT_STREQ(client.c_str(), "from2");
}

TEST_F(LinxQueueTests, getFdReturnefdFd) {
    auto queue = LinxQueueImpl(efdMock, 2);

    EXPECT_CALL(*efdMock, getFd()).WillOnce(Return(1));
    ASSERT_EQ(queue.getFd(), 1);
}
