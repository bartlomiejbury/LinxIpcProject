#include "gtest/gtest.h"
#include "AfUnixEndpoint.h"
#include "AfUnixServer.h"
#include "AfUnixSocketMock.h"
#include "LinxIpc.h"
#include "LinxQueueMock.h"

using namespace ::testing;

class AfUnixEndpointTests : public testing::Test {
  public:
    std::shared_ptr<AfUnixSocketMock> socket;
    AfUnixSocketMock* socketPtr;
    std::unique_ptr<LinxQueueMock> queue;
    LinxQueueMock* queuePtr;

    void SetUp() {
        socket = std::make_shared<NiceMock<AfUnixSocketMock>>();
        socketPtr = socket.get();
        ON_CALL(*socketPtr, open()).WillByDefault(Return(true));
        ON_CALL(*socketPtr, receive(_, _, _)).WillByDefault(Return(-1));
        ON_CALL(*socketPtr, send(_, _)).WillByDefault(Return(0));

        queue = std::make_unique<NiceMock<LinxQueueMock>>();
        queuePtr = queue.get();
        ON_CALL(*queuePtr, get(_, _, _)).WillByDefault(Return(ByMove(LinxReceivedMessagePtr(nullptr))));
        ON_CALL(*queuePtr, getFd()).WillByDefault(Return(1));
    }
};

TEST_F(AfUnixEndpointTests, receive_ReturnsNullWhenServerExpired) {
    std::weak_ptr<AfUnixServer> weakServer;

    {
        auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
        weakServer = server;
        // Server goes out of scope and is destroyed
    }

    // Create endpoint with expired server
    auto endpoint = std::make_shared<AfUnixEndpoint>(socket, weakServer, "TEST");

    auto result = endpoint->receive(1000, {1, 2, 3});
    ASSERT_EQ(result, nullptr);
}

TEST_F(AfUnixEndpointTests, receive_CallsServerReceiveWithSharedFromThis) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto endpoint = std::make_shared<AfUnixEndpoint>(socket, server, "TEST");

    auto sigsel = std::initializer_list<uint32_t>{4, 5};

    EXPECT_CALL(*queuePtr, get(1000, _, _))
        .WillOnce(Invoke([](int, const std::vector<uint32_t>&, const LinxReceiveContextOpt& from) {
            // Verify that from context is provided
            EXPECT_TRUE(from.has_value());
            return LinxReceivedMessagePtr(nullptr);
        }));

    endpoint->receive(1000, sigsel);
}

TEST_F(AfUnixEndpointTests, receive_ReturnsMessageWhenServerReturnsMessage) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto endpoint = std::make_shared<AfUnixEndpoint>(socket, server, "TEST");

    auto msg = std::make_unique<LinxReceivedMessage>(LinxReceivedMessage{
        .message = std::make_unique<LinxMessage>(42),
        .context = std::make_unique<AfUnixEndpoint>(socket, server, "TEST")
    });

    EXPECT_CALL(*queuePtr, get(_, _, _)).WillOnce(Invoke(
        [msg = std::move(msg)](int, const std::vector<uint32_t>&, const LinxReceiveContextOpt&) mutable {
            return std::move(msg);
        }
    ));

    auto result = endpoint->receive(1000, {42});
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 42U);
}

TEST_F(AfUnixEndpointTests, receive_ReturnsNullWhenServerReturnsNull) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto endpoint = std::make_shared<AfUnixEndpoint>(socket, server, "TEST");

    EXPECT_CALL(*queuePtr, get(_, _, _)).WillOnce(Return(ByMove(LinxReceivedMessagePtr(nullptr))));

    auto result = endpoint->receive(1000, {1, 2, 3});
    ASSERT_EQ(result, nullptr);
}

TEST_F(AfUnixEndpointTests, isEqual_ReturnsTrueWhenSameServerAndClient) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto endpoint1 = std::make_shared<AfUnixEndpoint>(socket, server, "TEST");
    auto endpoint2 = std::make_shared<AfUnixEndpoint>(socket, server, "TEST");

    ASSERT_TRUE(endpoint1->isEqual(*endpoint2));
}

TEST_F(AfUnixEndpointTests, isEqual_ReturnsFalseWhenDifferentServer) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = std::make_shared<NiceMock<AfUnixSocketMock>>();

    auto queue1 = std::make_unique<NiceMock<LinxQueueMock>>();
    auto queue2 = std::make_unique<NiceMock<LinxQueueMock>>();

    auto server1 = std::make_shared<AfUnixServer>(socket1, std::move(queue1), "TEST");
    auto server2 = std::make_shared<AfUnixServer>(socket2, std::move(queue2), "TEST");

    auto endpoint1 = std::make_shared<AfUnixEndpoint>(socket1, server1, "TEST");
    auto endpoint2 = std::make_shared<AfUnixEndpoint>(socket2, server2, "TEST");

    ASSERT_FALSE(endpoint1->isEqual(*endpoint2));
}

TEST_F(AfUnixEndpointTests, isEqual_ReturnsFalseWhenDifferentServiceName) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto endpoint1 = std::make_shared<AfUnixEndpoint>(socket, server, "TEST1");
    auto endpoint2 = std::make_shared<AfUnixEndpoint>(socket, server, "TEST2");

    ASSERT_FALSE(endpoint1->isEqual(*endpoint2));
}

TEST_F(AfUnixEndpointTests, isEqual_ReturnsFalseWhenOneServerExpired) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    std::weak_ptr<AfUnixServer> weakServer;

    {
        auto tempServer = std::make_shared<AfUnixServer>(
            std::make_shared<NiceMock<AfUnixSocketMock>>(),
            std::make_unique<NiceMock<LinxQueueMock>>(),
            "TEST"
        );
        weakServer = tempServer;
        // tempServer goes out of scope
    }

    auto endpoint1 = std::make_shared<AfUnixEndpoint>(socket, server, "TEST");
    auto endpoint2 = std::make_shared<AfUnixEndpoint>(socket, weakServer, "TEST");

    ASSERT_FALSE(endpoint1->isEqual(*endpoint2));
}

TEST_F(AfUnixEndpointTests, isEqual_ReturnsFalseWhenSameServerButDifferentSocket) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = std::make_shared<NiceMock<AfUnixSocketMock>>();

    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");

    auto endpoint1 = std::make_shared<AfUnixEndpoint>(socket1, server, "TEST");
    auto endpoint2 = std::make_shared<AfUnixEndpoint>(socket2, server, "TEST");

    ASSERT_FALSE(endpoint1->isEqual(*endpoint2));
}

TEST_F(AfUnixEndpointTests, isEqual_ReturnsTrueWhenBothServersExpired) {
    std::weak_ptr<AfUnixServer> weakServer1;
    std::weak_ptr<AfUnixServer> weakServer2;

    {
        auto tempServer = std::make_shared<AfUnixServer>(
            std::make_shared<NiceMock<AfUnixSocketMock>>(),
            std::make_unique<NiceMock<LinxQueueMock>>(),
            "TEST"
        );
        weakServer1 = tempServer;
        weakServer2 = tempServer;
        // tempServer goes out of scope, both weak_ptrs now expired
    }

    auto endpoint1 = std::make_shared<AfUnixEndpoint>(socket, weakServer1, "TEST");
    auto endpoint2 = std::make_shared<AfUnixEndpoint>(socket, weakServer2, "TEST");

    // Both server.lock() return nullptr, so nullptr == nullptr is true
    // And both use same socket and service name, so should be equal
    ASSERT_TRUE(endpoint1->isEqual(*endpoint2));
}

TEST_F(AfUnixEndpointTests, send_CallsParentClientSend) {
    auto server = std::make_shared<AfUnixServer>(socket, std::move(queue), "TEST");
    auto endpoint = std::make_shared<AfUnixEndpoint>(socket, server, "TEST");

    auto msg = LinxMessage(10);
    EXPECT_CALL(*socketPtr, send(Ref(msg), "TEST")).Times(1);

    endpoint->send(msg);
}
