#include <gmock/gmock.h>
#include <Utils.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class DatapointSerializerTest : public Test {
    public:
        uint8_t buffer[HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE];
        Datapoint datapoint;
        uint8_t actualLength;

    void SetUp() {
        datapoint = {};
    }

    void TearDown() {
        
    }
};

TEST_F(DatapointSerializerTest, SerializeBooleanDatapoint) {
    datapoint.id = 1;
    datapoint.type = DatapointType::BOOLEAN;
    datapoint.value_bool = true;

    actualLength = serializeDatapoint(&datapoint, buffer);
    
    Datapoint actualDatapoint = {};

    unserializeDatapoint(&actualDatapoint, buffer);

    ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 1));
    ASSERT_TRUE(actualDatapoint.value_bool);
}

TEST_F(DatapointSerializerTest, SerializeIntDatapoint) {
    int expected = -128123;
    datapoint.id = 1;
    datapoint.type = DatapointType::INTEGER;
    datapoint.value_int = expected;

    actualLength = serializeDatapoint(&datapoint, buffer);

    Datapoint actualDatapoint = {};

    unserializeDatapoint(&actualDatapoint, buffer);

    ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 4));
    ASSERT_THAT(actualDatapoint.value_int, Eq(expected));
}

TEST_F(DatapointSerializerTest, SerializeByteDatapoint) {
    uint8_t expected = 145;
    datapoint.id = 1;
    datapoint.type = DatapointType::BYTE;
    datapoint.value_byte = expected;

    actualLength = serializeDatapoint(&datapoint, buffer);

    Datapoint actualDatapoint = {};

    unserializeDatapoint(&actualDatapoint, buffer);

    ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 1));
    ASSERT_THAT(actualDatapoint.value_byte, Eq(expected));
}

TEST_F(DatapointSerializerTest, SerializeRawDatapoint) {
    uint8_t expected[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t raw[10];
    memcpy(raw, expected, 10);
    datapoint.id = 1;
    datapoint.type = DatapointType::RAW;
    datapoint.length = 10;
    datapoint.value_raw = raw;

    actualLength = serializeDatapoint(&datapoint, buffer);

    Datapoint actualDatapoint = {};
    uint8_t actualPayload[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    actualDatapoint.value_raw = actualPayload;

    unserializeDatapoint(&actualDatapoint, buffer);

    ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 10));
    ASSERT_THAT(actualDatapoint.value_raw, EqualToArray(expected, 10));
}

TEST_F(DatapointSerializerTest, SerializeStringDatapoint) {
    char expected[10];
    char valueString[10];
    memcpy(expected, "abcdefghi", 10);
    memcpy(valueString, expected, 10);
    datapoint.id = 1;
    datapoint.type = DatapointType::STRING;
    datapoint.value_string = valueString;
    datapoint.length = 10;

    actualLength = serializeDatapoint(&datapoint, buffer);

    Datapoint actualDatapoint = {};
    char actualPayload[10];
    actualDatapoint.value_string = actualPayload;

    unserializeDatapoint(&actualDatapoint, buffer);

    ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 10));
    ASSERT_THAT(actualDatapoint.value_string, StrEq(expected));
}
