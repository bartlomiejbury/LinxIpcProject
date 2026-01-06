#include <thread>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "UnixLinx.h"
#include "AfUnixSocketMock.h"
#include "LinxIpc.h"
#include "LinxMessageIds.h"

using namespace ::testing;

class AfUnixClientTests : public testing::Test {
  protected:
    std::unique_ptr<AfUnixSocketMock> socket;
    AfUnixSocketMock* socketPtr;

    void SetUp() override {
        socket = std::make_unique<NiceMock<AfUnixSocketMock>>();
        socketPtr = socket.get();
        ON_CALL(*socketPtr, receive(_, _, _)).WillByDefault(Return(-1));
        ON_CALL(*socketPtr, send(_, _)).WillByDefault(Return(0));
    }
};

TEST_F(AfUnixClientTests, send_CallSocketSend) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));
    auto msg = RawMessage(10);

    EXPECT_CALL(*socketPtr, send(Ref(msg), UnixInfo("TEST")));
    client.send(msg);
}

TEST_F(AfUnixClientTests, send_ReturnSocketSendResult) {
    auto client = std::make_shared<AfUnixClient>("test_instance", std::move(socket), UnixInfo("TEST"));
    auto msg = RawMessage(10);

    EXPECT_CALL(*socketPtr, send(_, _)).WillOnce(Return(2));
    ASSERT_EQ(client->send(msg), 2);
}

MATCHER_P(SigselMatcher, signals, "") {
    const std::vector<uint32_t> &sigsel = arg;
    const std::vector<uint32_t> expected = signals;
    return sigsel == expected;
}

TEST_F(AfUnixClientTests, receive_CallSocketReceive) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));
    std::initializer_list<uint32_t> sigsel = {2, 3};

    EXPECT_CALL(*socketPtr, receive(_, _, _));
    client.receive(1000, sigsel);
}

TEST_F(AfUnixClientTests, receive_ReturnSocketReceivedMsg) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(2);
            *from = std::make_unique<UnixInfo>("TEST");
            return 4;
        }
    ));
    auto result = client.receive(1000, {2, 3});
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 2);
}

TEST_F(AfUnixClientTests, receive_ReturnNullWhenFromDifferentService) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillRepeatedly(Invoke(
        [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            *msg = std::make_unique<RawMessage>(2);
            *from = std::make_unique<UnixInfo>("TEST2");
            return 4;
        }
    ));
    ASSERT_EQ(client.receive(100, {2, 3}), nullptr);
}

TEST_F(AfUnixClientTests, receive_ReturnNullWhenSignalNotInSigsel) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillRepeatedly(Invoke(
        [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            *msg = std::make_unique<RawMessage>(10);
            *from = std::make_unique<UnixInfo>("TEST");
            return 4;
        }
    ));
    ASSERT_EQ(client.receive(100, {2, 3}), nullptr);
}

TEST_F(AfUnixClientTests, receive_ReturnNullWhenSocketReturnError) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Return(-1));

    ASSERT_EQ(client.receive(10000, sigsel), nullptr);
}

TEST_F(AfUnixClientTests, receive_ReturnNullWhenSocketTimeout) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Return(0));

    ASSERT_EQ(client.receive(10000, sigsel), nullptr);
}

TEST_F(AfUnixClientTests, receive_ReturnMsgWhenSignalMatchAny) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(10);
            *from = std::make_unique<UnixInfo>("TEST");
            return 4;
        }
    ));
    auto result = client.receive(10000, LINX_ANY_SIG);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 10);
}

TEST_F(AfUnixClientTests, sendReceive_ReturnReturnNullPtr) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));
    auto msg = RawMessage(10);

    EXPECT_CALL(*socketPtr, send(_, _)).WillOnce(Return(-1));
    ASSERT_EQ(client.sendReceive(msg), nullptr);
}

TEST_F(AfUnixClientTests, sendReceive_ReturnCallserverSendAndReceive) {
    auto client =AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));
    auto msg = RawMessage(10);

    EXPECT_CALL(*socketPtr, send(Ref(msg), _)).WillOnce(Return(0));
    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillRepeatedly(Invoke(
        [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(12);
            *from = std::make_unique<UnixInfo>("TEST");
            return 4;
        }
    ));

    auto result = client.sendReceive(msg);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 12);
}

MATCHER_P(signalMatcher, reqid, "") {
    RawMessage &msg = (RawMessage &)arg;
    return msg.getReqId() == (uint32_t)reqid;
}

TEST_F(AfUnixClientTests, connect_SendHuntReq) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, send(signalMatcher(IPC_PING_REQ), _)).Times(1);
    client.connect(0);
}

TEST_F(AfUnixClientTests, connect_ReturnTrueWhenRspReceived) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(IPC_PING_RSP);
            *from = std::make_unique<UnixInfo>("TEST");
            return 4;
        }
    ));
    ASSERT_EQ(client.connect(0), true);
}

TEST_F(AfUnixClientTests, connect_NotCallReceiveWhenSendFail) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, send(_, _))
        .WillOnce(Return(-1))
        .WillOnce(Return(2));
    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(IPC_PING_RSP);
            *from = std::make_unique<UnixInfo>("TEST");
            return 4;
        }
    ));

    client.connect(10000);
}

TEST_F(AfUnixClientTests, connect_SendAgainWhenNoReponseReceived) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, send(signalMatcher(IPC_PING_REQ), _)).Times(2);
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .WillOnce(Invoke(
            [](RawMessagePtr*, std::unique_ptr<IIdentifier>*, int) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return -1;
            }
        ))
        .WillOnce(Invoke(
            [](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
                *msg = std::make_unique<RawMessage>(IPC_PING_RSP);
                *from = std::make_unique<UnixInfo>("TEST");
                return 4;
            }
        ));

    client.connect(INFINITE_TIMEOUT);
}

TEST_F(AfUnixClientTests, connect_ReturnFalseWhenTimeOutTimeout) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillRepeatedly(DoAll(
        InvokeWithoutArgs([]() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }),
        Return(-1)
    ));
    ASSERT_EQ(client.connect(1000), false);
}

TEST_F(AfUnixClientTests, testConnectDuration) {

    static constexpr int time = 1000;
    static constexpr int margin = 110;

    ON_CALL(*socketPtr, receive(_, _, _)).WillByDefault(DoAll(
        InvokeWithoutArgs([]() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); }),
        Return(-1)
    ));
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    auto startTime = std::chrono::steady_clock::now();
    ASSERT_FALSE(client.connect(time));
    auto endTime = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    ASSERT_GE(duration.count(), time-10) << "Connection should take at least " << time-10 << " ms";
    ASSERT_LE(duration.count(), time+margin) << "Connection should not take more than " << time+margin << " ms";
}

TEST_F(AfUnixClientTests, receive_LoopsContinueUntilCorrectMessageReceived) {
    auto client = AfUnixClient("test_instance", std::move(socket), UnixInfo("TEST"));

    // First call returns wrong signal (5 instead of 2 or 3)
    // Second call returns wrong service name
    // Third call returns correct message
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .WillOnce(Invoke([](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(5);  // Wrong signal
            *from = std::make_unique<UnixInfo>("TEST");
            return 4;
        }))
        .WillOnce(Invoke([](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(2);  // Right signal
            *from = std::make_unique<UnixInfo>("WRONG_SERVICE");  // Wrong service name
            return 4;
        }))
        .WillOnce(Invoke([](RawMessagePtr* msg, std::unique_ptr<IIdentifier>* from, int) {
            *msg = std::make_unique<RawMessage>(3);  // Right signal
            *from = std::make_unique<UnixInfo>("TEST");  // Right service name
            return 4;
        }));

    auto result = client.receive(10000, {2, 3});
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 3);
}

TEST_F(AfUnixClientTests, isEqual_ReturnsTrueWhenSameSocketAndSocketName) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = socket1; // Same shared_ptr

    auto client1 = AfUnixClient("test_instance", socket1, UnixInfo("TEST"));
    auto client2 = AfUnixClient("test_instance", socket2, UnixInfo("TEST"));

    ASSERT_TRUE(client1.isEqual(client2));
}

TEST_F(AfUnixClientTests, isEqual_ReturnsFalseWhenDifferentSocket) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = std::make_shared<NiceMock<AfUnixSocketMock>>();

    auto client1 = AfUnixClient("test_instance", socket1, UnixInfo("TEST"));
    auto client2 = AfUnixClient("test_instance", socket2, UnixInfo("TEST"));

    ASSERT_FALSE(client1.isEqual(client2));
}

TEST_F(AfUnixClientTests, isEqual_ReturnsFalseWhenDifferentServiceName) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = socket1; // Same shared_ptr

    auto client1 = AfUnixClient("test_instance", socket1, UnixInfo("TEST1"));
    auto client2 = AfUnixClient("test_instance", socket2, UnixInfo("TEST2"));

    ASSERT_FALSE(client1.isEqual(client2));
}
