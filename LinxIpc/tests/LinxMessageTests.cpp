#include "gtest/gtest.h"
#include "LinxIpc.h"

using namespace ::testing;

class LinxMessageTests : public testing::Test {};

TEST_F(LinxMessageTests, createEmptyMessage) {
    auto msg = LinxMessage(10);

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 0);
}

TEST_F(LinxMessageTests, createMessageFromInitializerList) {
    auto msg = LinxMessage(10, {1, 2, 3});

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 3);

    uint8_t expectedMessage[] = {1, 2, 3};
    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload<uint8_t *>(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxMessageTests, createMessageFromArray) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessage(10, expectedMessage);

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 4);

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload<uint8_t *>(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxMessageTests, createMessageFromStruct) {

    struct Data {
        int a;
        char b;
    } expectedMessage = {10, 5};

    auto msg = LinxMessage(10, expectedMessage);

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), sizeof(expectedMessage));

    ASSERT_EQ(memcmp((uint8_t *)(&expectedMessage), msg.getPayload(), sizeof(expectedMessage)), 0);

    struct Data *data = msg.getPayload<struct Data>();
    ASSERT_EQ(data->a, expectedMessage.a);
    ASSERT_EQ(data->b, expectedMessage.b);
}

TEST_F(LinxMessageTests, createMessageFromBuffer) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessage(10, expectedMessage, sizeof(expectedMessage));

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 4);

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload(), sizeof(expectedMessage)), 0);
}

TEST_F(LinxMessageTests, serializeDeserializeMessage) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessage(10, expectedMessage, sizeof(expectedMessage));

    uint8_t buffer[1024];
    auto serializedSizeOpt = msg.serialize(buffer, sizeof(buffer));
    ASSERT_TRUE(serializedSizeOpt.has_value());
    uint32_t serializedSize = serializedSizeOpt.value();

    auto  deserializedMsg = LinxMessage::deserialize(buffer, serializedSize);
    ASSERT_NE(deserializedMsg, nullptr);
    ASSERT_EQ(deserializedMsg->getReqId(), msg.getReqId());
    ASSERT_EQ(deserializedMsg->getPayloadSize(), msg.getPayloadSize());
    ASSERT_EQ(memcmp(deserializedMsg->getPayload(), msg.getPayload(), msg.getPayloadSize()), 0);
}

TEST_F(LinxMessageTests, serializeInsufficientBuffer) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = LinxMessage(10, expectedMessage, sizeof(expectedMessage));

    uint8_t buffer[4]; // Intentionally small buffer
    auto serializedSizeOpt = msg.serialize(buffer, sizeof(buffer));
    ASSERT_FALSE(serializedSizeOpt.has_value());
}

TEST_F(LinxMessageTests, testStructDataSerializeInsufficientBuffer) {

    struct Data {
        int a;
        char b;
        double c;
    } expectedMessage = {
        10,
        5,
        3.14
    };
    auto msg = LinxMessage(10, expectedMessage);
    ASSERT_EQ(msg.getPayloadSize(), sizeof(expectedMessage));

    ASSERT_EQ(msg.getPayload<Data>()->a, 10);
    ASSERT_EQ(msg.getPayload<Data>()->b, 5);
    ASSERT_EQ(msg.getPayload<Data>()->c, 3.14);

}
