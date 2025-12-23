#include <thread>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "AfUnixClient.h"
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
        ON_CALL(*socketPtr, open()).WillByDefault(Return(true));
    }
};

TEST_F(AfUnixClientTests, clienCallConnectOnCreation) {
    EXPECT_CALL(*socketPtr, open());
    auto client = AfUnixClient(std::move(socket), "TEST");
}

TEST_F(AfUnixClientTests, send_CallSocketSend) {
    auto client = AfUnixClient(std::move(socket), "TEST");
    auto msg = LinxMessage(10);

    EXPECT_CALL(*socketPtr, send(Ref(msg), "TEST"));
    client.send(msg);
}

TEST_F(AfUnixClientTests, send_ReturnSocketSendResult) {
    auto client = std::make_shared<AfUnixClient>(std::move(socket), "TEST");
    auto msg = LinxMessage(10);

    EXPECT_CALL(*socketPtr, send(_, _)).WillOnce(Return(2));
    ASSERT_EQ(client->send(msg), 2);
}

MATCHER_P(SigselMatcher, signals, "") {
    const std::vector<uint32_t> &sigsel = arg;
    const std::vector<uint32_t> expected = signals;
    return sigsel == expected;
}

TEST_F(AfUnixClientTests, receive_CallSocketReceive) {
    auto client = AfUnixClient(std::move(socket), "TEST");
    std::initializer_list<uint32_t> sigsel = {2, 3};

    EXPECT_CALL(*socketPtr, receive(_, _, _));
    client.receive(1000, sigsel);
}

TEST_F(AfUnixClientTests, receive_ReturnSocketReceivedMsg) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(2);
            *from = "TEST";
            return 0;
        }
    ));
    auto result = client.receive(1000, {2, 3});
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 2);
}

TEST_F(AfUnixClientTests, receive_ReturnNullWhenFromDifferentService) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillRepeatedly(Invoke(
        [](LinxMessagePtr* msg, std::string* from, int) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            *msg = std::make_unique<LinxMessage>(2);
            *from = "TEST2";
            return 0;
        }
    ));
    ASSERT_EQ(client.receive(100, {2, 3}), nullptr);
}

TEST_F(AfUnixClientTests, receive_ReturnNullWhenSignalNotInSigsel) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillRepeatedly(Invoke(
        [](LinxMessagePtr* msg, std::string* from, int) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            *msg = std::make_unique<LinxMessage>(10);
            *from = "TEST";
            return 0;
        }
    ));
    ASSERT_EQ(client.receive(100, {2, 3}), nullptr);
}

TEST_F(AfUnixClientTests, receive_ReturnNullWhenSocketReturnError) {
    auto client = AfUnixClient(std::move(socket), "TEST");
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Return(-1));

    ASSERT_EQ(client.receive(10000, sigsel), nullptr);
}

TEST_F(AfUnixClientTests, receive_ReturnMsgWhenSignalMatchAny) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(10);
            *from = "TEST";
            return 0;
        }
    ));
    auto result = client.receive(10000, LINX_ANY_SIG);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 10);
}

TEST_F(AfUnixClientTests, sendReceive_ReturnReturnNullPtr) {
    auto client = AfUnixClient(std::move(socket), "TEST");
    auto msg = LinxMessage(10);

    EXPECT_CALL(*socketPtr, send(_, _)).WillOnce(Return(-1));
    ASSERT_EQ(client.sendReceive(msg), nullptr);
}

TEST_F(AfUnixClientTests, sendReceive_ReturnCallserverSendAndReceive) {
    auto client =AfUnixClient(std::move(socket), "TEST");
    auto msg = LinxMessage(10);

    EXPECT_CALL(*socketPtr, send(Ref(msg), _)).WillOnce(Return(0));
    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillRepeatedly(Invoke(
        [](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(12);
            *from = "TEST";
            return 0;
        }
    ));

    auto result = client.sendReceive(msg);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 12);
}

MATCHER_P(signalMatcher, reqid, "") {
    LinxMessage &msg = (LinxMessage &)arg;
    return msg.getReqId() == (uint32_t)reqid;
}

TEST_F(AfUnixClientTests, connect_SendHuntReq) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, send(signalMatcher(IPC_PING_REQ), _)).Times(1);
    client.connect(0);
}

TEST_F(AfUnixClientTests, connect_ReturnTrueWhenRspReceived) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(IPC_PING_RSP);
            *from = "TEST";
            return 0;
        }
    ));
    ASSERT_EQ(client.connect(0), true);
}

TEST_F(AfUnixClientTests, connect_NotCallReceiveWhenSendFail) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, send(_, _))
        .WillOnce(Return(-1))
        .WillOnce(Return(2));
    EXPECT_CALL(*socketPtr, receive(_, _, _)).WillOnce(Invoke(
        [](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(IPC_PING_RSP);
            *from = "TEST";
            return 0;
        }
    ));

    client.connect(10000);
}

TEST_F(AfUnixClientTests, connect_SendAgainWhenNoReponseReceived) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    EXPECT_CALL(*socketPtr, send(signalMatcher(IPC_PING_REQ), _)).Times(2);
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .WillOnce(Invoke(
            [](LinxMessagePtr*, std::string*, int) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return -1;
            }
        ))
        .WillOnce(Invoke(
            [](LinxMessagePtr* msg, std::string* from, int) {
                *msg = std::make_unique<LinxMessage>(IPC_PING_RSP);
                *from = "TEST";
                return 0;
            }
        ));

    client.connect(INFINITE_TIMEOUT);
}

TEST_F(AfUnixClientTests, connect_ReturnFalseWhenTimeOutTimeout) {
    auto client = AfUnixClient(std::move(socket), "TEST");

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
    auto client = AfUnixClient(std::move(socket), "TEST");

    auto startTime = std::chrono::steady_clock::now();
    ASSERT_FALSE(client.connect(time));
    auto endTime = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    ASSERT_GE(duration.count(), time-10) << "Connection should take at least " << time-10 << " ms";
    ASSERT_LE(duration.count(), time+margin) << "Connection should not take more than " << time+margin << " ms";
}

TEST_F(AfUnixClientTests, receive_LoopsContinueUntilCorrectMessageReceived) {
    auto client = AfUnixClient(std::move(socket), "TEST");

    // First call returns wrong signal (5 instead of 2 or 3)
    // Second call returns wrong service name
    // Third call returns correct message
    EXPECT_CALL(*socketPtr, receive(_, _, _))
        .WillOnce(Invoke([](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(5);  // Wrong signal
            *from = "TEST";
            return 0;
        }))
        .WillOnce(Invoke([](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(2);  // Right signal
            *from = "WRONG_SERVICE";  // Wrong service name
            return 0;
        }))
        .WillOnce(Invoke([](LinxMessagePtr* msg, std::string* from, int) {
            *msg = std::make_unique<LinxMessage>(3);  // Right signal
            *from = "TEST";  // Right service name
            return 0;
        }));

    auto result = client.receive(10000, {2, 3});
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->getReqId(), 3);
}

TEST_F(AfUnixClientTests, isEqual_ReturnsTrueWhenSameSocketAndServiceName) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = socket1; // Same shared_ptr

    auto client1 = AfUnixClient(socket1, "TEST");
    auto client2 = AfUnixClient(socket2, "TEST");

    ASSERT_TRUE(client1.isEqual(client2));
}

TEST_F(AfUnixClientTests, isEqual_ReturnsFalseWhenDifferentSocket) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = std::make_shared<NiceMock<AfUnixSocketMock>>();

    auto client1 = AfUnixClient(socket1, "TEST");
    auto client2 = AfUnixClient(socket2, "TEST");

    ASSERT_FALSE(client1.isEqual(client2));
}

TEST_F(AfUnixClientTests, isEqual_ReturnsFalseWhenDifferentServiceName) {
    auto socket1 = std::make_shared<NiceMock<AfUnixSocketMock>>();
    auto socket2 = socket1; // Same shared_ptr

    auto client1 = AfUnixClient(socket1, "TEST1");
    auto client2 = AfUnixClient(socket2, "TEST2");

    ASSERT_FALSE(client1.isEqual(client2));
}

TEST_F(AfUnixClientTests, getName_ReturnsServiceName) {
    auto client = AfUnixClient(std::move(socket), "TEST_SERVICE");
    ASSERT_EQ(client.getName(), "TEST_SERVICE");
}
