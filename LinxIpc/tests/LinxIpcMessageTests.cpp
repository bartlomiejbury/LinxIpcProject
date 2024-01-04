
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpc.h"
#include "LinxIpcMessage.h"
#include "LinxIpcClientMock.h"

using namespace ::testing;

class LinxIpcMessageTests : public testing::Test {};

TEST_F(LinxIpcMessageTests, defaultConstructor) {
    auto msg = LinxMessageIpc();

    ASSERT_EQ(msg.getReqId(), 0);
    ASSERT_EQ(msg.getPayloadSize(), 0);
    ASSERT_EQ(msg.getClient(), nullptr);
}

TEST_F(LinxIpcMessageTests, createEmptyMessage) {
    auto msg = LinxMessageIpc(10);

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 0);
    ASSERT_EQ(msg.getClient(), nullptr);
}

TEST_F(LinxIpcMessageTests, createMessageFromInitializerList) {
    auto msg = LinxMessageIpc(10, {1, 2, 3});

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 3);
    ASSERT_EQ(msg.getClient(), nullptr);

    uint8_t expectedMessage[] = {1, 2, 3};
    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload<uint8_t *>(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxIpcMessageTests, createMessageFromArray) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessageIpc(10, expectedMessage);

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 4);
    ASSERT_EQ(msg.getClient(), nullptr);

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload<uint8_t *>(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxIpcMessageTests, createMessageFromStruct) {

    struct Data {
        int a;
        char b;
    } expectedMessage = {10, 5};

    auto msg = LinxMessageIpc(10, expectedMessage);

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), sizeof(expectedMessage));
    ASSERT_EQ(msg.getClient(), nullptr);

    ASSERT_EQ(memcmp((uint8_t *)(&expectedMessage), msg.getPayload(), sizeof(expectedMessage)), 0);

    struct Data *data = msg.getPayload<struct Data>();
    ASSERT_EQ(data->a, expectedMessage.a);
    ASSERT_EQ(data->b, expectedMessage.b);
}

TEST_F(LinxIpcMessageTests, createMessageFromBuffer) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessageIpc(10, expectedMessage, sizeof(expectedMessage));

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 4);
    ASSERT_EQ(msg.getClient(), nullptr);

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxIpcMessageTests, setClient) {

    LinxIpcClientPtr client = std::make_shared<LinxIpcClientMock>();

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessageIpc(10, expectedMessage, sizeof(expectedMessage));
    msg.setClient(client);

    ASSERT_EQ(msg.getClient(), client.get());
}
