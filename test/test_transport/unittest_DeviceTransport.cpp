#include <gmock/gmock.h>
#include <DeviceTransportUnderTest.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class DeviceTransportTest : public Test {
  public:
    DeviceTransportUnderTest *underTest;
    CommandPoolMock *commandPool;
    CommandQueueMock *commandQueue;
    NRFLiteMock *radio;
    uint8_t deviceAddress = 10;
    uint8_t hubAddress = 1;

  void SetUp() {
    radio = new NRFLiteMock();
    commandPool = new CommandPoolMock();
    commandQueue = new CommandQueueMock();
    underTest = new DeviceTransportUnderTest(deviceAddress, hubAddress, radio, commandPool, commandQueue);
  }

  void TearDown() {
    delete underTest;
    underTest = nullptr;

    delete radio;
    radio = nullptr;

    delete commandPool;
    commandPool = nullptr;

    delete commandQueue;
    commandQueue = nullptr;
  }
};

TEST_F(DeviceTransportTest, CallSendWhenSendingReportCommand) {
  uint8_t payload[4] = {1, 2, 3, 4};
  Command command = {};
  command.type = CommandType::DATAPOINT_REPORT;
  command.fromAddress = 10;
  command.toAddress = 1;
  command.payload = payload;
  command.payloadSize = 4;

  uint8_t expected[8];
  expected[0] = static_cast<uint8_t>(command.type);
  expected[1] = command.fromAddress;
  expected[2] = command.toAddress;
  expected[3] = command.payloadSize;
  memcpy(expected + HOMIO_COMMAND_HEADER_SIZE, payload, 4);

  EXPECT_CALL(*radio, send(1, EqualToArray(expected, 8), 8))
      .Times(AtLeast(1));

  underTest->sendCommand(&command);
}

TEST_F(DeviceTransportTest, CallSendWithCustomPayload) {
  uint8_t payload[5] = {1, 2, 3, 4, 5};
  Command command = {};
  command.type = CommandType::DATAPOINT_REPORT;
  command.fromAddress = 10;
  command.toAddress = 1;
  command.payload = payload;
  command.payloadSize = 5;

  uint8_t expected[9];
  expected[0] = static_cast<uint8_t>(command.type);
  expected[1] = command.fromAddress;
  expected[2] = command.toAddress;
  expected[3] = command.payloadSize;
  memcpy(expected + HOMIO_COMMAND_HEADER_SIZE, payload, 5);

  EXPECT_CALL(*radio, send(1, EqualToArray(expected, 9), 9))
      .Times(AtLeast(1));

  underTest->sendCommand(&command);
}

TEST_F(DeviceTransportTest, SendCommandToDifferentClient) {
  Command command = {};
  command.type = CommandType::DATAPOINT_REPORT;
  command.fromAddress = 2;
  command.toAddress = 3;

  uint8_t expected[4];
  expected[0] = static_cast<uint8_t>(command.type);
  expected[1] = command.fromAddress;
  expected[2] = command.toAddress;
  expected[3] = command.payloadSize;

  EXPECT_CALL(*radio, send(command.toAddress, EqualToArray(expected, 4), 4))
      .Times(AtLeast(1));

  underTest->sendCommand(&command);
}

TEST_F(DeviceTransportTest, SendCommandToCustomClient) {
  uint8_t customAddress = 2;
  Command command = {};
  command.toAddress = 1;

  EXPECT_CALL(*radio, send(customAddress, _, _))
      .Times(AtLeast(1));

  underTest->sendCommand(&command, customAddress);
}

TEST_F(DeviceTransportTest, ReturnTrueWhenSuccessfulSendCommand) {
  Command command = {};

  ON_CALL(*radio, send)
    .WillByDefault(Return(true));

  EXPECT_CALL(*radio, send);

  ASSERT_TRUE(underTest->sendCommand(&command));
}

TEST_F(DeviceTransportTest, ReturnFalseWhenUnsuccessulSendCommand) {
  Command commnad = {};

  ON_CALL(*radio, send)
    .WillByDefault(Return(0));

  EXPECT_CALL(*radio, send);

  ASSERT_FALSE(underTest->sendCommand(&commnad));
}

TEST_F(DeviceTransportTest, ReceiveConfirmCommand) {
  Command command = {};

  underTest->receiveCommand(&command);

  ASSERT_THAT(command.type, Eq(CommandType::CONFIRM));
}

TEST_F(DeviceTransportTest, AckReturnedNoData) {
  Command command = {};

  ON_CALL(*radio, hasAckData)
    .WillByDefault(Return(0));
  EXPECT_CALL(*radio, hasAckData());
  
  ASSERT_FALSE(underTest->receiveAck(&command));
}

TEST_F(DeviceTransportTest, AckReturnedConfirmCommand) {
  Command command = {};

  uint8_t expected[4];
  expected[0] = static_cast<uint8_t>(CommandType::CONFIRM);
  expected[1] = 1; // from
  expected[2] = 10; // to
  expected[3] = 0; // payload size

  ON_CALL(*radio, hasAckData)
    .WillByDefault(Return(1));

  EXPECT_CALL(*radio, hasAckData());
  EXPECT_CALL(*radio, readData(_))
    .WillOnce(Invoke([expected](void* data) {
      memcpy(data, expected, 4);
    }));

  underTest->receiveAck(&command);

  ASSERT_THAT(command.type, Eq(CommandType::CONFIRM));
}

TEST_F(DeviceTransportTest, AckReturnedDatapointDeliveredCommand) {
  Command command = {};

  uint8_t expected[4];
  expected[0] = static_cast<uint8_t>(CommandType::DATAPOINT_DELIVER);
  expected[1] = 1; // from
  expected[2] = 10; // to
  expected[3] = 0; // payload size

  ON_CALL(*radio, hasAckData)
    .WillByDefault(Return(1));
  ON_CALL(*radio, send)
    .WillByDefault(Return(1));

  EXPECT_CALL(*radio, hasAckData());
  EXPECT_CALL(*radio, readData(_))
    .WillOnce(Invoke([expected](void* data) {
      memcpy(data, expected, 4);
    }));

  underTest->receiveAck(&command);

  ASSERT_THAT(command, Field(&Command::type, Eq(CommandType::DATAPOINT_DELIVER)));
}