
#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "LinxIpcEndpointMock.h"
#include "LinxIpc.h"
#include "LinxIpcClientImpl.h"

using namespace ::testing;

class LinxIpcClientTests : public testing::Test {
  public:
    NiceMock<SystemMock> systemMock;
    std::shared_ptr<NiceMock<LinxIpcEndpointMock>> endpointMock = std::make_shared<NiceMock<LinxIpcEndpointMock>>();

    void SetUp() {
        struct timespec currentTime = {};
        ON_CALL(systemMock, usleep(_)).WillByDefault(Return(0));
        ON_CALL(systemMock, clock_gettime(_, _))
            .WillByDefault(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));
    }
};

TEST_F(LinxIpcClientTests, clientGetName) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");
    ASSERT_STREQ("TEST", client->getName().c_str());
}

TEST_F(LinxIpcClientTests, send) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*endpointMock.get(), send(Ref(msg), _)).WillOnce(Return(2));

    int result = client->send(msg);
    ASSERT_EQ(result, 2);
}

TEST_F(LinxIpcClientTests, receive) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");
    auto msg = std::make_shared<LinxMessageIpc>(10);
    std::initializer_list<uint32_t> sigsel = {2, 3};

    EXPECT_CALL(*endpointMock.get(), receive(1000, Ref(sigsel), _)).WillOnce(Return(msg));

    auto result = client->receive(1000, sigsel);
    ASSERT_EQ(result, msg);
}

MATCHER_P(signakMatcher, reqid, "") {
    LinxMessageIpc &msg = (LinxMessageIpc &)arg;
    return msg.getReqId() == (uint32_t)reqid;
}

TEST_F(LinxIpcClientTests, connect_ReceivedResponse) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");

    EXPECT_CALL(*endpointMock.get(), send(signakMatcher(IPC_HUNT_REQ), _)).Times(1).WillRepeatedly(Return(2));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*endpointMock.get(), receive(100, _, _)).WillOnce(Return(msg));

    auto result = client->connect(0);
    ASSERT_EQ(result, true);
}

TEST_F(LinxIpcClientTests, connect_ErrorDuringSend) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");

    EXPECT_CALL(*endpointMock.get(), send(signakMatcher(IPC_HUNT_REQ), _)).WillOnce(Return(-1)).WillOnce(Return(2));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*endpointMock.get(), receive(100, _, _)).WillOnce(Return(msg));

    auto result = client->connect(10000);
    ASSERT_EQ(result, true);
}

TEST_F(LinxIpcClientTests, connect_ReceivedWrongMessage) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");

    EXPECT_CALL(*endpointMock.get(), send(signakMatcher(IPC_HUNT_REQ), _)).Times(2).WillRepeatedly(Return(2));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*endpointMock.get(), receive(100, _, _)).WillOnce(Return(nullptr)).WillOnce(Return(msg));

    auto result = client->connect(10000);
    ASSERT_EQ(result, true);
}

TEST_F(LinxIpcClientTests, connect_InfiniteTimeout) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");

    EXPECT_CALL(*endpointMock.get(), send(signakMatcher(IPC_HUNT_REQ), _)).Times(2).WillRepeatedly(Return(2));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*endpointMock.get(), receive(100, _, _)).WillOnce(Return(nullptr)).WillOnce(Return(msg));

    auto result = client->connect(INFINITE_TIMEOUT);
    ASSERT_EQ(result, true);
}

TEST_F(LinxIpcClientTests, connect_Timeout) {
    auto client = std::make_shared<LinxIpcClientImpl>(endpointMock, "TEST");

    EXPECT_CALL(*endpointMock.get(), send(signakMatcher(IPC_HUNT_REQ), _)).Times(2).WillRepeatedly(Return(2));

    auto msg = std::make_shared<LinxMessageIpc>(10);
    EXPECT_CALL(*endpointMock.get(), receive(100, _, _)).Times(2).WillRepeatedly(Return(nullptr));

    struct timespec currentTime1 = {.tv_sec = 1, .tv_nsec = 900000000};
    struct timespec currentTime2 = {.tv_sec = 11, .tv_nsec = 0};
    struct timespec currentTime3 = {.tv_sec = 12, .tv_nsec = 0};
    EXPECT_CALL(systemMock, clock_gettime(_, _))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime1, &currentTime1 + 1), Return(0)))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime2, &currentTime2 + 1), Return(0)))
        .WillOnce(DoAll(SetArrayArgument<1>(&currentTime3, &currentTime3 + 1), Return(0)));

    auto result = client->connect(10000);
    ASSERT_EQ(result, false);
}
