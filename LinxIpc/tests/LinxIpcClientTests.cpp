
#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "LinxIpcServerMock.h"
#include "LinxIpc.h"
#include "LinxIpcClientImpl.h"

using namespace ::testing;

class LinxIpcClientTests : public testing::Test {
  public:
    NiceMock<SystemMock> systemMock;
    std::shared_ptr<NiceMock<LinxIpcServerMock>> serverMock = std::make_shared<NiceMock<LinxIpcServerMock>>();
    struct timespec currentTime = {};

    void SetUp() {
        ON_CALL(systemMock, usleep(_)).WillByDefault(Return(0));
        ON_CALL(systemMock, clock_gettime(_, _))
            .WillByDefault(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));

        auto msg = std::make_shared<LinxMessageIpc>(10);
        ON_CALL(*serverMock.get(), receive(100, _, _)).WillByDefault(Return(msg));
        ON_CALL(*serverMock.get(), send(_, _)).WillByDefault(Return(0));
    }
};

TEST_F(LinxIpcClientTests, clientGetName) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    ASSERT_STREQ(client->getName().c_str(), "TEST");
}

TEST_F(LinxIpcClientTests, send_CallEndpointSend) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*serverMock.get(), send(Ref(msg), _));
    client->send(msg);
}

TEST_F(LinxIpcClientTests, send_ReturnEndpointSendResult) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*serverMock.get(), send(_, _)).WillOnce(Return(2));
    ASSERT_EQ(client->send(msg), 2);
}

MATCHER_P(SigselMatcher, signals, "") {
    const std::vector<uint32_t> &sigsel = arg;
    const std::vector<uint32_t> expected = signals;
    return sigsel == expected;
}

TEST_F(LinxIpcClientTests, receive_CallEndpointReceive) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    std::initializer_list<uint32_t> sigsel = {2, 3};

    EXPECT_CALL(*serverMock.get(), receive(1000, SigselMatcher(sigsel), _));
    client->receive(1000, sigsel);
}

TEST_F(LinxIpcClientTests, receive_ReturnEndpointReceivedMsg) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*serverMock.get(), receive(_, _, _)).WillOnce(Return(msg));
    ASSERT_EQ(client->receive(1000, {2, 3}), msg);
}

MATCHER_P(signalMatcher, reqid, "") {
    LinxMessageIpc &msg = (LinxMessageIpc &)arg;
    return msg.getReqId() == (uint32_t)reqid;
}

TEST_F(LinxIpcClientTests, connect_SendHuntReq) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");

    EXPECT_CALL(*serverMock.get(), send(signalMatcher(IPC_HUNT_REQ), _)).Times(1);
    client->connect(0);
}

TEST_F(LinxIpcClientTests, connect_ReturnTrueWhenRspReceived) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*serverMock.get(), receive(100, _, _)).WillOnce(Return(msg));
    ASSERT_EQ(client->connect(0), true);
}

TEST_F(LinxIpcClientTests, connect_NotCallReceiveWhenSendFail) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*serverMock.get(), send(_, _))
        .WillOnce(Return(-1))
        .WillOnce(Return(2));
    EXPECT_CALL(*serverMock.get(), receive(100, _, _)).WillOnce(Return(msg));

    client->connect(10000);
}

TEST_F(LinxIpcClientTests, connect_SendAgainWhenNoReponseReceived) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*serverMock.get(), send(signalMatcher(IPC_HUNT_REQ), _)).Times(2);
    EXPECT_CALL(*serverMock.get(), receive(100, _, _))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(msg));

    client->connect(INFINITE_TIMEOUT);
}

TEST_F(LinxIpcClientTests, connect_ReturnFalseWhenTimeOutTimeout) {
    auto client = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*serverMock.get(), receive(100, _, _)).Times(2).WillRepeatedly(Return(nullptr));

    struct timespec currentTime1 = {.tv_sec = 1, .tv_nsec = 900000000};
    struct timespec currentTime2 = {.tv_sec = 11, .tv_nsec = 0};
    struct timespec currentTime3 = {.tv_sec = 12, .tv_nsec = 0};
    EXPECT_CALL(systemMock, clock_gettime(_, _))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime1, &currentTime1 + 1), Return(0)))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime2, &currentTime2 + 1), Return(0)))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime3, &currentTime3 + 1), Return(0)));

    ASSERT_EQ(client->connect(10000), false);
}

TEST_F(LinxIpcClientTests, operaorEquality) {
    auto client1 = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto client2 = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST");
    auto client3 = std::make_shared<LinxIpcClientImpl>(serverMock, "TEST2");

    ASSERT_TRUE(*client1 ==*client2);
    ASSERT_FALSE(*client1 ==*client3);

    ASSERT_FALSE(*client1 !=*client2);
    ASSERT_TRUE(*client1 !=*client3);
}
