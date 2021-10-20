#include <gmock/gmock.h>
#include <transport.h>
#include <matchers.h>

using namespace ::testing;
using namespace Homio;

class DatapointSerializerTest : public Test {
  public:
    Transport *transport;
    NRFLiteMock *radio;
    uint8_t buffer[HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE];
    Datapoint datapoint;
    uint8_t actualLength;

  void SetUp() {
    radio = new NRFLiteMock();
    transport = new Transport(radio);
    datapoint = {};
  }

  void TearDown() {
    delete transport;
    transport = nullptr;

    delete radio;
    radio = nullptr;
  }
};

TEST_F(DatapointSerializerTest, SerializeSelectsExpectedDatapoint) {
  const uint8_t expectedDatapointId = 10;
  datapoint.id = expectedDatapointId;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  transport->addDatapoint(&datapoint);
  transport->serializeDatapoint(expectedDatapointId, buffer);

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(expectedDatapoint, Field(&Datapoint::id, Eq(expectedDatapointId)));
}

TEST_F(DatapointSerializerTest, SerializeSelectsSecondDatapointIfMany) {
  const uint8_t expectedDatapointId = 10;
  Datapoint datapointA;
  datapointA.id = 1;
  datapointA.type = DatapointType::BOOLEAN;
  datapointA.value_bool = true;

  Datapoint datapointB;
  datapointB.id = expectedDatapointId;
  datapointB.type = DatapointType::BOOLEAN;
  datapointB.value_bool = false;

  transport->addDatapoint(&datapointA);
  transport->addDatapoint(&datapointB);
  transport->serializeDatapoint(expectedDatapointId, buffer);

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(expectedDatapoint, Field(&Datapoint::id, Eq(expectedDatapointId)));
}

TEST_F(DatapointSerializerTest, SerializeBooleanDatapoint) {
  datapoint.id = 1;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  transport->addDatapoint(&datapoint);
  actualLength = transport->serializeDatapoint(1, buffer);

  datapoint.value_int = 0;

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 1));
  ASSERT_TRUE(expectedDatapoint->value_bool);
}

TEST_F(DatapointSerializerTest, SerializeIntDatapoint) {
  int expected = -128123;
  datapoint.id = 1;
  datapoint.type = DatapointType::INTEGER;
  datapoint.value_int = expected;

  transport->addDatapoint(&datapoint);
  actualLength = transport->serializeDatapoint(1, buffer);

  datapoint.value_int = 0;

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 4));
  ASSERT_THAT(expectedDatapoint->value_int, Eq(expected));
}

TEST_F(DatapointSerializerTest, SerializeByteDatapoint) {
  uint8_t expected = 145;
  datapoint.id = 1;
  datapoint.type = DatapointType::BYTE;
  datapoint.value_byte = expected;

  transport->addDatapoint(&datapoint);
  actualLength = transport->serializeDatapoint(1, buffer);

  datapoint.value_int = 0;

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 1));
  ASSERT_THAT(expectedDatapoint->value_byte, Eq(expected));
}

TEST_F(DatapointSerializerTest, SerializeRawDatapoint) {
  uint8_t expected[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  uint8_t raw[10];
  memcpy(raw, expected, 10);
  datapoint.id = 1;
  datapoint.type = DatapointType::RAW;
  datapoint.length = 10;
  datapoint.value_raw = raw;

  transport->addDatapoint(&datapoint);
  actualLength = transport->serializeDatapoint(1, buffer);

  memset(raw, 0, 10);

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 10));
  ASSERT_THAT(expectedDatapoint->value_raw, EqualToArray(expected, 10));
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

  transport->addDatapoint(&datapoint);
  actualLength = transport->serializeDatapoint(1, buffer);

  memset(valueString, 0, 10);

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 10));
  ASSERT_THAT(expectedDatapoint->value_string, StrEq(expected));
}

TEST_F(DatapointSerializerTest, FailToDeserializeNonexistingDatapoint) {
  datapoint.id = 1;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  transport->addDatapoint(&datapoint);
  transport->serializeDatapoint(1, buffer);

  buffer[0] = 2;

  Datapoint *expectedDatapoint = transport->unserializeDatapoint(buffer);

  ASSERT_THAT(expectedDatapoint, Eq(nullptr));
}

TEST_F(DatapointSerializerTest, SerailizerReturnsEmptyIfNoDatapointToSerialize) {
  datapoint.id = 1;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  ASSERT_THAT(transport->serializeDatapoint(1, buffer), Eq(0));
}