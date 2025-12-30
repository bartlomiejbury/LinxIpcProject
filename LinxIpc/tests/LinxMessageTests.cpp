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

    // Test with buffer that's too small - need at least 8 bytes (4 reqId + 4 payload)
    // Use 7 bytes (just 1 byte short) to test insufficient buffer without triggering compiler warnings
    std::vector<uint8_t> buffer(7);  // Intentionally too small by 1 byte
    uint32_t serializedSize = msg.serialize(buffer.data(), buffer.size());
    ASSERT_EQ(serializedSize, 0);
}

TEST_F(RawMessageTests, createMessageWithPayloadSize) {
    auto msg = RawMessage(42, 10);

    ASSERT_EQ(msg.getReqId(), 42);
    ASSERT_EQ(msg.getPayloadSize(), 10);

    // Payload should be allocated and initialized to zeros
    const uint8_t *payload = msg.getPayload();
    for (size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(payload[i], 0);
    }
}

TEST_F(RawMessageTests, createMessageFromVector) {
    std::vector<uint8_t> buffer = {5, 6, 7, 8, 9};
    auto msg = RawMessage(33, buffer);

    ASSERT_EQ(msg.getReqId(), 33);
    ASSERT_EQ(msg.getPayloadSize(), 5);

    const uint8_t *payload = msg.getPayload();
    for (size_t i = 0; i < buffer.size(); ++i) {
        ASSERT_EQ(payload[i], buffer[i]);
    }
}

TEST_F(RawMessageTests, createMessageFromVectorMove) {
    std::vector<uint8_t> buffer = {10, 20, 30, 40};
    auto msg = RawMessage(55, std::move(buffer));

    ASSERT_EQ(msg.getReqId(), 55);
    ASSERT_EQ(msg.getPayloadSize(), 4);

    const uint8_t expected[] = {10, 20, 30, 40};
    ASSERT_EQ(memcmp(expected, msg.getPayload(), sizeof(expected)), 0);
}

TEST_F(RawMessageTests, deserializeInsufficientBuffer) {
    // Create a buffer that's too small (less than 4 bytes for reqId)
    std::vector<uint8_t> buffer = {1, 2};

    auto msg = RawMessage::deserialize(std::move(buffer));

    ASSERT_EQ(msg, nullptr);
}

