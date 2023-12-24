
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpc.h"
#include "LinxIpcMessage.h"
#include "LinxIpcClientMock.h"

using namespace ::testing;

class LinxIpcMessageTests : public testing::Test {};

TEST_F(LinxIpcMessageTests, defaultConstructor) {
    auto msg = LinxMessageIpc();

    ASSERT_EQ(0, msg.getReqId());
    ASSERT_EQ(0, msg.getPayloadSize());
    ASSERT_EQ(nullptr, msg.getClient());
}

TEST_F(LinxIpcMessageTests, emptyMessage) {
    auto msg = LinxMessageIpc(10);

    ASSERT_EQ(10, msg.getReqId());
    ASSERT_EQ(0, msg.getPayloadSize());
    ASSERT_EQ(nullptr, msg.getClient());
}

TEST_F(LinxIpcMessageTests, messageFromInitializerList) {
    auto msg = LinxMessageIpc(10, {1, 2, 3});

    ASSERT_EQ(10, msg.getReqId());
    ASSERT_EQ(3, msg.getPayloadSize());
    ASSERT_EQ(nullptr, msg.getClient());

    uint8_t expectedMessage[] = {1, 2, 3};
    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload<uint8_t *>(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxIpcMessageTests, messageFromArray) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessageIpc(10, expectedMessage);

    ASSERT_EQ(10, msg.getReqId());
    ASSERT_EQ(4, msg.getPayloadSize());
    ASSERT_EQ(nullptr, msg.getClient());

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload<uint8_t *>(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxIpcMessageTests, messageFromStruct) {

    struct Data {
        int a;
        char b;
    } expectedMessage = {10, 5};

    auto msg = LinxMessageIpc(10, expectedMessage);

    ASSERT_EQ(10, msg.getReqId());
    ASSERT_EQ(sizeof(expectedMessage), msg.getPayloadSize());
    ASSERT_EQ(nullptr, msg.getClient());

    ASSERT_EQ(memcmp((uint8_t *)(&expectedMessage), msg.getPayload(), sizeof(expectedMessage)), 0);

    struct Data *data = msg.getPayload<struct Data>();
    ASSERT_EQ(data->a, expectedMessage.a);
    ASSERT_EQ(data->b, expectedMessage.b);
}

TEST_F(LinxIpcMessageTests, messageFromBuffer) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessageIpc(10, expectedMessage, sizeof(expectedMessage));

    ASSERT_EQ(10, msg.getReqId());
    ASSERT_EQ(4, msg.getPayloadSize());
    ASSERT_EQ(nullptr, msg.getClient());

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxIpcMessageTests, setClient) {

    LinxIpcClientPtr client = std::make_shared<LinxIpcClientMock>();

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessageIpc(10, expectedMessage, sizeof(expectedMessage));
    msg.setClient(client);

    ASSERT_EQ(msg.getClient(), client.get());
}
