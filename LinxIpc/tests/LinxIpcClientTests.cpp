
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcServerMock.h"
#include "LinxIpc.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcPrivate.h"

using namespace ::testing;

class LinxIpcClientTests : public testing::Test {
  public:
    std::shared_ptr<NiceMock<LinxIpcServerMock>> server = std::make_shared<NiceMock<LinxIpcServerMock>>();

    void SetUp() {
        auto msg = std::make_shared<LinxMessageIpc>(10);
        ON_CALL(*server.get(), receive(_, _, _)).WillByDefault(Return(msg));
        ON_CALL(*server.get(), send(_, _)).WillByDefault(Return(0));
    }
};

TEST_F(LinxIpcClientTests, clientGetName) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    ASSERT_STREQ(client->getName().c_str(), "TEST");
}

TEST_F(LinxIpcClientTests, send_CallEndpointSend) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*server.get(), send(Ref(msg), _));
    client->send(msg);
}

TEST_F(LinxIpcClientTests, send_ReturnEndpointSendResult) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*server.get(), send(_, _)).WillOnce(Return(2));
    ASSERT_EQ(client->send(msg), 2);
}

MATCHER_P(SigselMatcher, signals, "") {
    const std::vector<uint32_t> &sigsel = arg;
    const std::vector<uint32_t> expected = signals;
    return sigsel == expected;
}

TEST_F(LinxIpcClientTests, receive_CallEndpointReceive) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    std::initializer_list<uint32_t> sigsel = {2, 3};

    EXPECT_CALL(*server.get(), receive(1000, SigselMatcher(sigsel), _));
    client->receive(1000, sigsel);
}

TEST_F(LinxIpcClientTests, receive_ReturnEndpointReceivedMsg) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*server.get(), receive(_, _, _)).WillOnce(Return(msg));
    ASSERT_EQ(client->receive(1000, {2, 3}), msg);
}

TEST_F(LinxIpcClientTests, sendReceive_ReturnReturnNullPtr) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*server.get(), send(_, _)).WillOnce(Return(-1));
    ASSERT_EQ(client->sendReceive(msg), nullptr);
}

TEST_F(LinxIpcClientTests, sendReceive_ReturnCallserverSendAndReceive) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = LinxMessageIpc(10);
    auto rsp = std::make_shared<LinxMessageIpc>(12);

    EXPECT_CALL(*server.get(), send(Ref(msg), _)).WillOnce(Return(0));
    EXPECT_CALL(*server.get(), receive(INFINITE_TIMEOUT, SigselMatcher(LINX_ANY_SIG), _)).WillOnce(Return(rsp));

    ASSERT_EQ(client->sendReceive(msg), rsp);
}

MATCHER_P(signalMatcher, reqid, "") {
    LinxMessageIpc &msg = (LinxMessageIpc &)arg;
    return msg.getReqId() == (uint32_t)reqid;
}

TEST_F(LinxIpcClientTests, connect_SendHuntReq) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");

    EXPECT_CALL(*server.get(), send(signalMatcher(IPC_PING_REQ), _)).Times(1);
    client->connect(0);
}

TEST_F(LinxIpcClientTests, connect_ReturnTrueWhenRspReceived) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*server.get(), receive(100, _, _)).WillOnce(Return(msg));
    ASSERT_EQ(client->connect(0), true);
}

TEST_F(LinxIpcClientTests, connect_NotCallReceiveWhenSendFail) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*server.get(), send(_, _))
        .WillOnce(Return(-1))
        .WillOnce(Return(2));
    EXPECT_CALL(*server.get(), receive(100, _, _)).WillOnce(Return(msg));

    client->connect(10000);
}

TEST_F(LinxIpcClientTests, connect_SendAgainWhenNoReponseReceived) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*server.get(), send(signalMatcher(IPC_PING_REQ), _)).Times(2);
    EXPECT_CALL(*server.get(), receive(100, _, _))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(msg));

    client->connect(INFINITE_TIMEOUT);
}

TEST_F(LinxIpcClientTests, connect_ReturnFalseWhenTimeOutTimeout) {
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*server.get(), receive(100, _, _)).WillRepeatedly(Return(nullptr));
    ASSERT_EQ(client->connect(1000), false);
}

TEST_F(LinxIpcClientTests, operaorEquality) {
    auto client1 = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto client2 = std::make_shared<LinxIpcClientImpl>(server, "TEST");
    auto client3 = std::make_shared<LinxIpcClientImpl>(server, "TEST2");

    ASSERT_TRUE(*client1 ==*client2);
    ASSERT_FALSE(*client1 ==*client3);

    ASSERT_FALSE(*client1 !=*client2);
    ASSERT_TRUE(*client1 !=*client3);
}

TEST_F(LinxIpcClientTests, testConnectDuration) {

    static constexpr int time = 1000;
    static constexpr int margin = 110;

    ON_CALL(*server.get(), receive(_, _, _)).WillByDefault(Return(nullptr));
    auto client = std::make_shared<LinxIpcClientImpl>(server, "TEST");

    auto startTime = std::chrono::steady_clock::now();
    ASSERT_FALSE(client->connect(time));
    auto endTime = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    ASSERT_GE(duration.count(), time-10) << "Connection should take at least " << time-10 << " ms";
    ASSERT_LE(duration.count(), time+margin) << "Connection should not take more than " << time+margin << " ms";
}
