#include "gtest/gtest.h"
#include "LinxIpc.h"
#include <arpa/inet.h>

using namespace ::testing;

class RawMessageTests : public testing::Test {};

TEST_F(RawMessageTests, createEmptyMessage) {
    auto msg = RawMessage(10);

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 0);
}

TEST_F(RawMessageTests, createMessageFromInitializerList) {
    auto msg = RawMessage(10, {1, 2, 3});

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 3);

    uint8_t expectedMessage[] = {1, 2, 3};
    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload(), sizeof(expectedMessage)), 0);
}

TEST_F(RawMessageTests, createMessageFromArray) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = RawMessage(10, expectedMessage, sizeof(expectedMessage));

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 4);

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload(), sizeof(expectedMessage)), 0);
}

TEST_F(RawMessageTests, testStructDataSerializeInsufficientBuffer) {

    struct Data {
        int a;
        char b;
        double c;
    } expectedMessage = {
        10,
        5,
        3.14
    };
    auto msg = ILinxMessage<Data>(10, expectedMessage);
    ASSERT_EQ(msg.getPayloadSize(), sizeof(expectedMessage));

    const struct Data *actualPayload = msg.getPayload();
    ASSERT_EQ(actualPayload->a, 10);
    ASSERT_EQ(actualPayload->b, 5);
    ASSERT_EQ(actualPayload->c, 3.14);
}

TEST_F(RawMessageTests, createMessageFromBuffer) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = RawMessage(10, expectedMessage, sizeof(expectedMessage));

    ASSERT_EQ(msg.getReqId(), 10);
    ASSERT_EQ(msg.getPayloadSize(), 4);

    ASSERT_EQ(memcmp(expectedMessage, msg.getPayload(), sizeof(expectedMessage)), 0);
}

TEST_F(RawMessageTests, serializeDeserializeMessage) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = RawMessage(10, expectedMessage, sizeof(expectedMessage));

    uint8_t buffer[1024];
    uint32_t serializedSize = msg.serialize(buffer, sizeof(buffer));
    ASSERT_NE(serializedSize, 0);

    // Convert buffer to vector and move it to deserialize
    std::vector<uint8_t> vec(buffer, buffer + serializedSize);
    auto  deserializedMsg = RawMessage::deserialize(std::move(vec));
    ASSERT_NE(deserializedMsg, nullptr);
    ASSERT_EQ(deserializedMsg->getReqId(), msg.getReqId());
    ASSERT_EQ(deserializedMsg->getPayloadSize(), msg.getPayloadSize());
    ASSERT_EQ(memcmp(deserializedMsg->getPayload(), msg.getPayload(), msg.getPayloadSize()), 0);
}

TEST_F(RawMessageTests, serializeInsufficientBuffer) {

    uint8_t expectedMessage[] = {1, 2, 3, 3};
    auto msg = RawMessage(10, expectedMessage, sizeof(expectedMessage));

    uint8_t buffer[4]; // Intentionally small buffer
    uint32_t serializedSize = msg.serialize(buffer, sizeof(buffer));
    ASSERT_EQ(serializedSize, 0);
}

