
#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "LinxIpc.h"
#include "LinxIpcEventFdImpl.h"

using namespace ::testing;

class LinxIpcEventFdTests : public testing::Test {
  public:
    NiceMock<SystemMock> systemMock;

    void SetUp() {
        ON_CALL(systemMock, eventfd(_, _)).WillByDefault(Return(0));
        ON_CALL(systemMock, close(_)).WillByDefault(Return(0));
    }
};

TEST_F(LinxIpcEventFdTests, Close_DoNothingWhenFailedToCreate) {

    EXPECT_CALL(systemMock, eventfd(_, _)).WillOnce(Return(-1));
    EXPECT_CALL(systemMock, close(_)).Times(0);
    {
        auto eventFd = LinxIpcEventFdImpl();
    }
}

TEST_F(LinxIpcEventFdTests, Close_CallCloseWhenCreateSuccess) {

    EXPECT_CALL(systemMock, eventfd(_, _)).WillOnce(Return(0));
    EXPECT_CALL(systemMock, close(_)).Times(1);
    {
        auto eventFd = LinxIpcEventFdImpl();
    }
}

TEST_F(LinxIpcEventFdTests, getFd_ReturnCorrectFd) {

    EXPECT_CALL(systemMock, eventfd(_, _)).WillOnce(Return(1));
    auto eventFd = LinxIpcEventFdImpl();

    ASSERT_EQ(eventFd.getFd(), 1);
}

TEST_F(LinxIpcEventFdTests, WriteEvent_ReturnErrorWhenFailedToCreate) {

    EXPECT_CALL(systemMock, eventfd(_, _)).WillOnce(Return(-1));
    auto eventFd = LinxIpcEventFdImpl();

    ASSERT_EQ(eventFd.writeEvent(), -1);
}

TEST_F(LinxIpcEventFdTests, WriteEvent_ReturnErrorWhenFailedToWrite) {

    EXPECT_CALL(systemMock, write(_, _, _)).WillOnce(Return(4));
    auto eventFd = LinxIpcEventFdImpl();

    ASSERT_EQ(eventFd.writeEvent(), -2);
}

TEST_F(LinxIpcEventFdTests, WriteEvent_CallWrite) {

    EXPECT_CALL(systemMock, write(_, _, 8));
    auto eventFd = LinxIpcEventFdImpl();
    eventFd.writeEvent();
}

TEST_F(LinxIpcEventFdTests, WriteEvent_ReturnSuccessWhenWriteOk) {

    EXPECT_CALL(systemMock, write(_, _, _)).WillOnce(Return(sizeof(uint64_t)));
    auto eventFd = LinxIpcEventFdImpl();

    ASSERT_EQ(eventFd.writeEvent(), 0);
}

TEST_F(LinxIpcEventFdTests, ReadEvent_ReturnErrorWhenFailedToCreate) {

    EXPECT_CALL(systemMock, eventfd(_, _)).WillOnce(Return(-1));
    auto eventFd = LinxIpcEventFdImpl();

    ASSERT_EQ(eventFd.readEvent(), -1);
}

TEST_F(LinxIpcEventFdTests, ReadEvent_ReturnErrorWhenFailedToRead) {

    EXPECT_CALL(systemMock, read(_, _, _)).WillOnce(Return(4));
    auto eventFd = LinxIpcEventFdImpl();

    ASSERT_EQ(eventFd.readEvent(), -2);
}

TEST_F(LinxIpcEventFdTests, ReadEvent_CallRead) {

    EXPECT_CALL(systemMock, read(_, _, 8));
    auto eventFd = LinxIpcEventFdImpl();
    eventFd.readEvent();
}

TEST_F(LinxIpcEventFdTests, ReadEvent_ReturnSuccessWhenReadOk) {

    EXPECT_CALL(systemMock, read(_, _, _)).WillOnce(Return(sizeof(uint64_t)));
    auto eventFd = LinxIpcEventFdImpl();

    ASSERT_EQ(eventFd.readEvent(), 0);
}

TEST_F(LinxIpcEventFdTests, ClearEvents_CallReadUntilFail) {

    EXPECT_CALL(systemMock, read(_, _, _))
        .WillOnce(Return(sizeof(uint64_t)))
        .WillOnce(Return(sizeof(uint64_t)))
        .WillOnce(Return(-1));

    auto eventFd = LinxIpcEventFdImpl();
    eventFd.clearEvents();
}

TEST_F(LinxIpcEventFdTests, ClearEvents_NotReadWhenCreateFail) {

    EXPECT_CALL(systemMock, eventfd(_, _)).WillOnce(Return(-1));
    EXPECT_CALL(systemMock, read(_, _, _)).Times(0);

    auto eventFd = LinxIpcEventFdImpl();
    eventFd.clearEvents();
}
