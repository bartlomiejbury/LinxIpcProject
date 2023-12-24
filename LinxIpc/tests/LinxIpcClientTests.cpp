
#include <stdio.h>
#include "gtest/gtest.h"
#include "SystemMock.h"
#include "LinxIpcEndpointMock.h"
#include "LinxIpc.h"
#include "LinxIpcCLientImpl.h"

using namespace ::testing;

class LinxIpcClientTests : public testing::Test {
  public:
    NiceMock<SystemMock> systemMock;
    std::shared_ptr<NiceMock<LinxIpcEndpointMock>> endpointMock = std::make_shared<NiceMock<LinxIpcEndpointMock>>();

    void SetUp() {
        struct timespec currentTime = {};
        ON_CALL(systemMock, clock_gettime(_, _))
            .WillByDefault(DoAll(SetArrayArgument<1>(&currentTime, &currentTime + 1), Return(0)));
    }
};

TEST_F(LinxIpcClientTests, clientGetName) {
    auto client = LinxIpcClientImpl(endpointMock, "TEST");

    ASSERT_STREQ("TEST", client.getName().c_str());
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
