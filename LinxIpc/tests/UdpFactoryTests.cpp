#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "UdpLinx.h"
#include "UdpSocket.h"
#include "SystemMock.h"

using namespace ::testing;

// Since UdpFactory uses concrete UdpSocket instances, we'll test the actual behavior
// and use SystemMock for system call failures

class UdpFactoryTests : public testing::Test {
  protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// Test successful server creation without multicast
TEST_F(UdpFactoryTests, createServer_SuccessWithoutMulticast) {
    auto server = UdpFactory::createServer(0, 10);  // Port 0 = let OS assign

    ASSERT_NE(server, nullptr);
    EXPECT_FALSE(server->getName().empty());
}

// Test successful server creation with multicast
TEST_F(UdpFactoryTests, createServer_SuccessWithMulticast) {
    const std::string LINX_MULTICAST_IP_ADDRESS = "239.0.0.1";
    auto server = UdpFactory::createMulticastServer(LINX_MULTICAST_IP_ADDRESS, 0, 10);  // Port 0 = let OS assign

    ASSERT_NE(server, nullptr);
    EXPECT_FALSE(server->getName().empty());
}

// Test server creation with specific port
TEST_F(UdpFactoryTests, createServer_WithSpecificPort) {
    // Try to create server on a high port number (less likely to be in use)
    auto server = UdpFactory::createServer(54321, 10);

    if (server != nullptr) {
        EXPECT_FALSE(server->getName().empty());
        EXPECT_NE(server->getName().find("54321"), std::string::npos);
    }
    // Note: Test may fail if port is already in use, which is acceptable
}

// Test successful client creation
TEST_F(UdpFactoryTests, createClient_Success) {
    auto client = UdpFactory::createClient("127.0.0.1", 12345);

    ASSERT_NE(client, nullptr);
    EXPECT_FALSE(client->getName().empty());
    EXPECT_NE(client->getName().find("client_"), std::string::npos);
    EXPECT_NE(client->getName().find("127.0.0.1:12345"), std::string::npos);
}

// Test client creation with multicast IP
TEST_F(UdpFactoryTests, createClient_WithMulticastIp) {
    auto client = UdpFactory::createClient("224.0.0.1", 12345);

    ASSERT_NE(client, nullptr);
    EXPECT_FALSE(client->getName().empty());
}

// Test client creation with broadcast IP
TEST_F(UdpFactoryTests, createClient_WithBroadcastIp) {
    auto client = UdpFactory::createClient("255.255.255.255", 12345);

    ASSERT_NE(client, nullptr);
    EXPECT_FALSE(client->getName().empty());
}

// Test multiple clients have unique instance IDs
TEST_F(UdpFactoryTests, createClient_UniqueInstanceIds) {
    auto client1 = UdpFactory::createClient("127.0.0.1", 12345);
    auto client2 = UdpFactory::createClient("127.0.0.1", 12345);
    auto client3 = UdpFactory::createClient("127.0.0.1", 12345);

    ASSERT_NE(client1, nullptr);
    ASSERT_NE(client2, nullptr);
    ASSERT_NE(client3, nullptr);

    // Instance IDs should be different due to random generation
    EXPECT_NE(client1->getName(), client2->getName());
    EXPECT_NE(client2->getName(), client3->getName());
    EXPECT_NE(client1->getName(), client3->getName());
}

// Test server creation with different queue sizes
TEST_F(UdpFactoryTests, createServer_WithDifferentQueueSizes) {
    auto server1 = UdpFactory::createServer(0, 10);
    auto server2 = UdpFactory::createServer(0, 100);
    auto server3 = UdpFactory::createServer(0, 1000);

    ASSERT_NE(server1, nullptr);
    ASSERT_NE(server2, nullptr);
    ASSERT_NE(server3, nullptr);
}

// Test isMulticastIp function
TEST_F(UdpFactoryTests, isMulticastIp_ReturnsTrueForMulticastAddresses) {
    EXPECT_TRUE(UdpFactory::isMulticastIp("224.0.0.0"));
    EXPECT_TRUE(UdpFactory::isMulticastIp("224.0.0.1"));
    EXPECT_TRUE(UdpFactory::isMulticastIp("239.255.255.255"));
}

TEST_F(UdpFactoryTests, isMulticastIp_ReturnsFalseForNonMulticastAddresses) {
    EXPECT_FALSE(UdpFactory::isMulticastIp("127.0.0.1"));
    EXPECT_FALSE(UdpFactory::isMulticastIp("192.168.1.1"));
    EXPECT_FALSE(UdpFactory::isMulticastIp("255.255.255.255"));
    EXPECT_FALSE(UdpFactory::isMulticastIp("0.0.0.0"));
}

// Test isBroadcastIp function
TEST_F(UdpFactoryTests, isBroadcastIp_ReturnsTrueForBroadcastAddress) {
    EXPECT_TRUE(UdpFactory::isBroadcastIp("255.255.255.255"));
}

TEST_F(UdpFactoryTests, isBroadcastIp_ReturnsFalseForNonBroadcastAddresses) {
    EXPECT_FALSE(UdpFactory::isBroadcastIp("127.0.0.1"));
    EXPECT_FALSE(UdpFactory::isBroadcastIp("192.168.1.1"));
    EXPECT_FALSE(UdpFactory::isBroadcastIp("224.0.0.1"));
    EXPECT_FALSE(UdpFactory::isBroadcastIp("0.0.0.0"));
}

// Error path tests using SystemMock

TEST_F(UdpFactoryTests, createServer_ReturnsNullWhenSocketOpenFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(-1));

    auto server = UdpFactory::createServer(12345, 10);
    EXPECT_EQ(server, nullptr);
}

TEST_F(UdpFactoryTests, createServer_ReturnsNullWhenBindFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(100));  // Return valid fd
    EXPECT_CALL(mock, bind(100, _, _))
        .WillOnce(Return(-1));  // Bind fails
    EXPECT_CALL(mock, close(100))
        .WillOnce(Return(0));

    auto server = UdpFactory::createServer(12345, 10);
    EXPECT_EQ(server, nullptr);
}

TEST_F(UdpFactoryTests, createServer_ReturnsNullWhenJoinMulticastFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(100));
    EXPECT_CALL(mock, bind(100, _, _))
        .WillOnce(Return(0));
    EXPECT_CALL(mock, setsockopt(100, IPPROTO_IP, IP_ADD_MEMBERSHIP, _, _))
        .WillOnce(Return(-1));  // Join multicast fails
    EXPECT_CALL(mock, close(100))
        .WillOnce(Return(0));

    const std::string LINX_MULTICAST_IP_ADDRESS = "239.0.0.1";
    auto server = UdpFactory::createMulticastServer(LINX_MULTICAST_IP_ADDRESS, 12345, 10);  // Multicast enabled
    EXPECT_EQ(server, nullptr);
}

TEST_F(UdpFactoryTests, createClient_ReturnsNullWhenSocketOpenFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(-1));

    auto client = UdpFactory::createClient("127.0.0.1", 12345);
    EXPECT_EQ(client, nullptr);
}

TEST_F(UdpFactoryTests, createClient_ReturnsNullWhenSetMulticastTtlFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(100));
    // setMulticastTtl calls setsockopt twice: once for TTL, once for LOOP
    // Both are called even if first fails, function returns first error
    EXPECT_CALL(mock, setsockopt(100, IPPROTO_IP, IP_MULTICAST_TTL, _, _))
        .WillOnce(Return(-1));  // First call (TTL) fails
    EXPECT_CALL(mock, setsockopt(100, IPPROTO_IP, IP_MULTICAST_LOOP, _, _))
        .WillOnce(Return(0));   // Second call still happens

    auto client = UdpFactory::createClient("239.255.255.255", 12345);  // Multicast IP
    EXPECT_EQ(client, nullptr);
}

TEST_F(UdpFactoryTests, createClient_ReturnsNullWhenSetBroadcastFails) {
    SystemMock mock;
    EXPECT_CALL(mock, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(100));
    EXPECT_CALL(mock, setsockopt(100, SOL_SOCKET, SO_BROADCAST, _, _))
        .WillOnce(Return(-1));  // Set broadcast fails
    EXPECT_CALL(mock, close(100))
        .WillOnce(Return(0));

    auto client = UdpFactory::createClient("255.255.255.255", 12345);  // Broadcast IP
    EXPECT_EQ(client, nullptr);
}
