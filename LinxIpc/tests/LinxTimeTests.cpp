#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "LinxTime.h"

using namespace ::testing;

TEST(LinxTimeTest, getTimeMs) {
    NiceMock<SystemMock> systemMock;

    struct timespec currentTime = {};
    ON_CALL(systemMock, clock_gettime(_, _))
        .WillByDefault(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));

    currentTime = {.tv_sec = 1, .tv_nsec = 500000000};
    uint64_t time = getTimeMs();
    ASSERT_EQ(time, 1500);

    currentTime = {.tv_sec = 2, .tv_nsec = 1000000};
    time = getTimeMs();
    ASSERT_EQ(time, 2001);
}

TEST(LinxTimeTest, timeMsToTimespec) {
    struct timespec currentTime = {};

    currentTime = timeMsToTimespec(1500);
    ASSERT_EQ(currentTime.tv_sec, 1);
    ASSERT_EQ(currentTime.tv_nsec, 500000000);

    currentTime = timeMsToTimespec(2001);
    ASSERT_EQ(currentTime.tv_sec, 2);
    ASSERT_EQ(currentTime.tv_nsec, 1000000);
}
