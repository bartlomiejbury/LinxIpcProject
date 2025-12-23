#include <thread>
#include "gtest/gtest.h"
#include "Deadline.h"

TEST(DeadlineTests, Constructor_WithNormalTimeout_SetsDeadline) {
    Deadline deadline(1000);
    ASSERT_FALSE(deadline.isExpired());
}

TEST(DeadlineTests, Constructor_WithInfiniteTimeout_NeverExpires) {
    Deadline deadline(INFINITE_TIMEOUT);
    ASSERT_FALSE(deadline.isExpired());
}

TEST(DeadlineTests, isExpired_ReturnsFalseBeforeTimeout) {
    Deadline deadline(100);
    ASSERT_FALSE(deadline.isExpired());
}

TEST(DeadlineTests, isExpired_ReturnsTrueAfterTimeout) {
    Deadline deadline(10);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_TRUE(deadline.isExpired());
}

TEST(DeadlineTests, isExpired_NeverExpiresForInfiniteTimeout) {
    Deadline deadline(INFINITE_TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_FALSE(deadline.isExpired());
}

TEST(DeadlineTests, getRemainingTimeMs_ReturnsInfiniteForInfiniteTimeout) {
    Deadline deadline(INFINITE_TIMEOUT);
    ASSERT_EQ(deadline.getRemainingTimeMs(), INFINITE_TIMEOUT);
}

TEST(DeadlineTests, getRemainingTimeMs_ReturnsPositiveValueBeforeExpiry) {
    Deadline deadline(1000);
    int remaining = deadline.getRemainingTimeMs();
    ASSERT_GT(remaining, 0);
    ASSERT_LE(remaining, 1000);
}

TEST(DeadlineTests, getRemainingTimeMs_ReturnsZeroAfterExpiry) {
    Deadline deadline(5);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(deadline.getRemainingTimeMs(), 0);
}

TEST(DeadlineTests, getRemainingTimeMs_DecreasesOverTime) {
    Deadline deadline(200);
    int remaining1 = deadline.getRemainingTimeMs();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    int remaining2 = deadline.getRemainingTimeMs();

    ASSERT_GT(remaining1, remaining2);
}

TEST(DeadlineTests, ZeroTimeout_ExpiresImmediately) {
    Deadline deadline(0);
    // With zero timeout, it might be expired or very close to expiry
    int remaining = deadline.getRemainingTimeMs();
    ASSERT_LE(remaining, 1);
}
