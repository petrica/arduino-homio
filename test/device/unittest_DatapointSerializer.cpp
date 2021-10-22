#include <gmock/gmock.h>
#include <component.h>
#include <matchers.h>

using namespace ::testing;
using namespace Homio;

class Homio::ComponentUnderTest: public Component {
  public:
    uint8_t serializeDatapoint(uint8_t datapointId, uint8_t *buffer) {
      return Component::serializeDatapoint(datapointId, buffer);
    }

    Datapoint *unserializeDatapoint(uint8_t *buffer) {
      return Component::unserializeDatapoint(buffer);
    }

    uint8_t getDatapointsCount() {
      return datapointsCount_;
    }
};

class DatapointSerializerTest : public Test {
  public:
    ComponentUnderTest *underTest;
    uint8_t buffer[HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE];
    Datapoint datapoint;
    uint8_t actualLength;

  void SetUp() {
    underTest = new ComponentUnderTest();
    datapoint = {};
  }

  void TearDown() {
    delete underTest;
    underTest = nullptr;
  }
};

TEST_F(DatapointSerializerTest, DatapointsListIsEmpty) {
  uint8_t listCount = underTest->getDatapointsCount();
  ASSERT_THAT(listCount, Eq(0));
}

TEST_F(DatapointSerializerTest, DatapointsListHasCountOfOne) {
  Datapoint datapoint;
  underTest->addDatapoint(&datapoint);
  uint8_t listCount = underTest->getDatapointsCount();
  ASSERT_THAT(listCount, Eq(1));
}

TEST_F(DatapointSerializerTest, SerializeSelectsExpectedDatapoint) {
  const uint8_t expectedDatapointId = 10;
  datapoint.id = expectedDatapointId;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  underTest->addDatapoint(&datapoint);
  underTest->serializeDatapoint(expectedDatapointId, buffer);

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

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

  underTest->addDatapoint(&datapointA);
  underTest->addDatapoint(&datapointB);
  underTest->serializeDatapoint(expectedDatapointId, buffer);

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

  ASSERT_THAT(expectedDatapoint, Field(&Datapoint::id, Eq(expectedDatapointId)));
}

TEST_F(DatapointSerializerTest, SerializeBooleanDatapoint) {
  datapoint.id = 1;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  underTest->addDatapoint(&datapoint);
  actualLength = underTest->serializeDatapoint(1, buffer);

  datapoint.value_int = 0;

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 1));
  ASSERT_TRUE(expectedDatapoint->value_bool);
}

TEST_F(DatapointSerializerTest, SerializeIntDatapoint) {
  int expected = -128123;
  datapoint.id = 1;
  datapoint.type = DatapointType::INTEGER;
  datapoint.value_int = expected;

  underTest->addDatapoint(&datapoint);
  actualLength = underTest->serializeDatapoint(1, buffer);

  datapoint.value_int = 0;

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 4));
  ASSERT_THAT(expectedDatapoint->value_int, Eq(expected));
}

TEST_F(DatapointSerializerTest, SerializeByteDatapoint) {
  uint8_t expected = 145;
  datapoint.id = 1;
  datapoint.type = DatapointType::BYTE;
  datapoint.value_byte = expected;

  underTest->addDatapoint(&datapoint);
  actualLength = underTest->serializeDatapoint(1, buffer);

  datapoint.value_int = 0;

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

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

  underTest->addDatapoint(&datapoint);
  actualLength = underTest->serializeDatapoint(1, buffer);

  memset(raw, 0, 10);

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

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

  underTest->addDatapoint(&datapoint);
  actualLength = underTest->serializeDatapoint(1, buffer);

  memset(valueString, 0, 10);

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

  ASSERT_THAT(actualLength, Eq(HOMIO_DATAPOINT_HEADER_SIZE + 10));
  ASSERT_THAT(expectedDatapoint->value_string, StrEq(expected));
}

TEST_F(DatapointSerializerTest, FailToDeserializeNonexistingDatapoint) {
  datapoint.id = 1;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  underTest->addDatapoint(&datapoint);
  underTest->serializeDatapoint(1, buffer);

  buffer[0] = 2;

  Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

  ASSERT_THAT(expectedDatapoint, Eq(nullptr));
}

TEST_F(DatapointSerializerTest, SerailizerReturnsEmptyIfNoDatapointToSerialize) {
  datapoint.id = 1;
  datapoint.type = DatapointType::BOOLEAN;
  datapoint.value_bool = true;

  ASSERT_THAT(underTest->serializeDatapoint(1, buffer), Eq(0));
}