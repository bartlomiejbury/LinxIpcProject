
#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "PthreadMock.h"
#include "LinxIpc.h"
#include "LinxQueueImpl.h"

using namespace ::testing;

class LinxQueueTests : public testing::Test {
   public:
    NiceMock<SystemMock> systemMock;
    NiceMock<PthreadMock> pthreadMock;

    void SetUp() {
        struct timespec currentTime = {};
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

TEST_F(LinxQueueTests, addToQueue_MaximumSizeReached) {
    auto queue = LinxQueueImpl(1);

    LinxMessageIpcPtr msg{};
    ASSERT_EQ(0, queue.add(msg, "from"));
    ASSERT_EQ(-1, queue.add(msg, "from"));
}

TEST_F(LinxQueueTests, clearQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg{};
    queue.add(msg, "from");
    queue.add(msg, "from");

    ASSERT_EQ(2, queue.size());

    queue.clear();
    ASSERT_EQ(0, queue.size());
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(nullptr, queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt));
}

TEST_F(LinxQueueTests, get_Immediate_NotGetElementFromQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(2, queue.size());

    queue.get(IMMEDIATE_TIMEOUT, {3, 4}, std::nullopt);
    ASSERT_EQ(2, queue.size());
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto msg = queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}

TEST_F(LinxQueueTests, get_Immediate_GetElementFromQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(2, queue.size());

    queue.get(IMMEDIATE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_EQ(1, queue.size());
}

TEST_F(LinxQueueTests, get_Immediate_ReturnNullWhenNoSignalSenderInQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(nullptr, queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from3")));
}

TEST_F(LinxQueueTests, get_Immediate_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto msg = queue.get(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}

TEST_F(LinxQueueTests, get_Infinite_WaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(2);

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

    auto msg = queue.get(INFINITE_TIMEOUT, {2, 3}, std::nullopt);
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto msg = queue.get(INFINITE_TIMEOUT, {3, 2}, std::nullopt);
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}

TEST_F(LinxQueueTests, get_Infinite_WaitWhenNoSignalSenderInQueue) {
    auto queue = LinxQueueImpl(2);

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

    auto msg = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}

TEST_F(LinxQueueTests, get_Infinite_ReturnMsgWhenSignalSenderInQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto msg = queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}

TEST_F(LinxQueueTests, get_Infinite_GetElementFromQueueWhenSignalArrive) {
    auto queue = LinxQueueImpl(2);

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

    ASSERT_EQ(0, queue.size());
    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    ASSERT_EQ(1, queue.size());
}

TEST_F(LinxQueueTests, get_Infinite_GetElementFromQueue) {
    auto queue = LinxQueueImpl(2);

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    ASSERT_EQ(2, queue.size());
    queue.get(INFINITE_TIMEOUT, LINX_ANY_SIG, std::make_optional("from2"));
    ASSERT_EQ(1, queue.size());
}

MATCHER_P2(TimespecMatcher, sec, nsec, "") {
    struct timespec *currentTime = (struct timespec *)arg;
    return currentTime->tv_sec == sec && currentTime->tv_nsec == nsec;
}

TEST_F(LinxQueueTests, get_Timeout_WaitWhenNoSignalNrInQueue) {
    auto queue = LinxQueueImpl(2);

    struct timespec currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(systemMock, clock_gettime(_, _))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));

    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, TimespecMatcher(2, 200000000)))
        .WillOnce([&queue]() { return 0; })
        .WillOnce([&queue]() {
            LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
            queue.add(msg2, "from2");
            return 0;
        });

    auto msg = queue.get(500, {2, 3}, std::nullopt);
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}

TEST_F(LinxQueueTests, get_Timeout_ReturnNullWhenWaitTimedOut) {
    auto queue = LinxQueueImpl(2);

    struct timespec currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(systemMock, clock_gettime(_, _))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));

    EXPECT_CALL(pthreadMock, pthread_cond_timedwait(_, _, TimespecMatcher(2, 200000000))).WillOnce([&queue]() {
        return ETIMEDOUT;
    });

    ASSERT_EQ(nullptr, queue.get(500, {2, 3}, std::nullopt));
}

TEST_F(LinxQueueTests, get_Timeout_ReturnMsgWhenSignalNrInQueue) {
    auto queue = LinxQueueImpl(2);

    struct timespec currentTime = {.tv_sec = 1, .tv_nsec = 700000000};
    EXPECT_CALL(systemMock, clock_gettime(_, _))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));

    LinxMessageIpcPtr msg1 = std::make_shared<LinxMessageIpc>(1);
    queue.add(msg1, "from1");

    LinxMessageIpcPtr msg2 = std::make_shared<LinxMessageIpc>(2);
    queue.add(msg2, "from2");

    auto msg = queue.get(500, {3, 2}, std::nullopt);
    ASSERT_EQ(2, msg->message->getReqId());
    ASSERT_STREQ("from2", msg->from.c_str());
}
