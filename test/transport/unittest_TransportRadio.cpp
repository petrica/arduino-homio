#include <gmock/gmock.h>
#include <transport.h>
#include <matchers.h>

using namespace ::testing;
using namespace Homio;

class TransportTest : public Test {
  public:
    Transport *transport;
    NRFLiteMock *radio;

  void SetUp() {
    radio = new NRFLiteMock();
    transport = new Transport(radio);
  }

  void TearDown() {
    delete transport;
    transport = nullptr;

    delete radio;
    radio = nullptr;
  }
};

TEST_F(TransportTest, DatapointsListIsEmpty) {
  uint8_t listCount = transport->getDatapointsCount();
  ASSERT_THAT(listCount, Eq(0));
}

TEST_F(TransportTest, DatapointsListHasCountOfOne) {
  Datapoint datapoint;
  transport->addDatapoint(&datapoint);
  uint8_t listCount = transport->getDatapointsCount();
  ASSERT_THAT(listCount, Eq(1));
}

TEST_F(TransportTest, CallSendWhenSendingHeartbeatCommand) {
  Command command = {};
  command.type = CommandType::HEARTBEAT;
  command.fromAddress = 10;
  command.toAddress = 1;
  command.payload = {};
  command.payloadSize = 0;
  
  uint8_t expected[4];
  expected[0] = static_cast<uint8_t>(command.type);
  expected[1] = command.fromAddress;
  expected[2] = command.toAddress;
  expected[3] = command.payloadSize;

  EXPECT_CALL(*radio, send(1, EqualToArray(expected, 4), 4))
      .Times(AtLeast(1));

  transport->sendCommand(&command);
}

TEST_F(TransportTest, CallSendWhenSendingReportCommand) {
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

  transport->sendCommand(&command);
}

TEST_F(TransportTest, CallSendWithCustomPayload) {
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

  transport->sendCommand(&command);
}

TEST_F(TransportTest, SendCommandToDifferentClient) {
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

  transport->sendCommand(&command);
}

TEST_F(TransportTest, SendCommandToCustomClient) {
  uint8_t customAddress = 2;
  Command command = {};
  command.toAddress = 1;

  EXPECT_CALL(*radio, send(customAddress, _, _))
      .Times(AtLeast(1));

  transport->sendCommand(&command, customAddress);
}

TEST_F(TransportTest, ReturnTrueWhenSuccessfulSendCommand) {
  Command command = {};

  ON_CALL(*radio, send)
    .WillByDefault(Return(true));

  EXPECT_CALL(*radio, send);

  ASSERT_TRUE(transport->sendCommand(&command));
}

TEST_F(TransportTest, ReturnFalseWhenUnsuccessulSendCommand) {
  Command commnad = {};

  ON_CALL(*radio, send)
    .WillByDefault(Return(0));

  EXPECT_CALL(*radio, send);

  ASSERT_FALSE(transport->sendCommand(&commnad));
}

TEST_F(TransportTest, ReceiveConfirmCommand) {
  Command command = {};

  transport->receiveCommand(&command);

  ASSERT_THAT(command.type, Eq(CommandType::CONFIRM));
}

TEST_F(TransportTest, AckReturnedNoData) {
  Command command = {};

  ON_CALL(*radio, hasAckData)
    .WillByDefault(Return(0));
  EXPECT_CALL(*radio, hasAckData());
  
  ASSERT_FALSE(transport->receiveAck(&command));
}

TEST_F(TransportTest, AckReturnedConfirmCommand) {
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

  transport->receiveAck(&command);

  ASSERT_THAT(command.type, Eq(CommandType::CONFIRM));
}

TEST_F(TransportTest, AckReturnedDatapointDeliveredCommand) {
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

  transport->receiveAck(&command);

  ASSERT_THAT(command, Field(&Command::type, Eq(CommandType::DATAPOINT_DELIVER)));
}