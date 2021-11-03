#include <gmock/gmock.h>
#include <DeviceProtocol.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class DeviceProtocolTest : public Test {
  public:
    DeviceProtocol *underTest;
    TransportMock *transport;
    Command commandHeartbeat = {};

  void SetUp() {
    transport = new TransportMock();
    underTest = new DeviceProtocol(transport);

    commandHeartbeat.type = CommandType::HEARTBEAT;
    commandHeartbeat.fromAddress = 10;
    commandHeartbeat.toAddress = 1;
  }

  void TearDown() {
    delete underTest;
    underTest = nullptr;

    delete transport;
    transport = nullptr;
  }
};

TEST_F(DeviceProtocolTest, SuccessfullyProcessIfAllConditionsMet) {
  Command expectedCommand = {};
  expectedCommand.type = CommandType::CONFIRM;
  expectedCommand.fromAddress = 1;
  expectedCommand.toAddress = 10;
  expectedCommand.payloadSize = 0;

  Command receivedCommand = {};

  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(true));
  EXPECT_CALL(*transport, receiveAck(_)) 
    .WillOnce(Invoke([=](const Command *command) -> bool { 
      memcpy((void*)command, &expectedCommand, sizeof(Command));
      return true; 
    }));
  EXPECT_CALL(*transport, sendCommand(_));

  ASSERT_TRUE(underTest->processCommand(&commandHeartbeat, &receivedCommand));
}

TEST_F(DeviceProtocolTest, FailProcessIfNoAckData) {
  Command receivedCommand = {};
  
  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(true));
  ON_CALL(*transport, receiveAck)
    .WillByDefault(Return(false));

  EXPECT_CALL(*transport, sendCommand(_));
  EXPECT_CALL(*transport, receiveAck(_));

  ASSERT_FALSE(underTest->processCommand(&commandHeartbeat, &receivedCommand));
}

TEST_F(DeviceProtocolTest, FailProcessIfSendFailed) {
  Command receivedCommand = {};

  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(false));

  EXPECT_CALL(*transport, sendCommand(_));
  EXPECT_CALL(*transport, receiveAck(_))
    .Times(0);

  ASSERT_FALSE(underTest->processCommand(&commandHeartbeat, &receivedCommand));
}

TEST_F(DeviceProtocolTest, FailProcessIfAckDataIntendedForOtherDevice) {
  Command expectedCommand = {};
  expectedCommand.type = CommandType::CONFIRM;
  expectedCommand.fromAddress = 1;
  expectedCommand.toAddress = 11;
  expectedCommand.payloadSize = 0;

  Command receivedCommand = {};

  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(true));
  EXPECT_CALL(*transport, receiveAck(_)) 
    .WillOnce(Invoke([=](const Command *command) -> bool { 
      memcpy((void*)command, &expectedCommand, sizeof(Command));
      return true; 
    }));
  EXPECT_CALL(*transport, sendCommand(_));

  ASSERT_FALSE(underTest->processCommand(&commandHeartbeat, &receivedCommand));
}

TEST_F(DeviceProtocolTest, ReceiveConfirmCommand) {
  Command expectedCommand = {};
  expectedCommand.type = CommandType::CONFIRM;
  expectedCommand.fromAddress = 1;
  expectedCommand.toAddress = 11;
  expectedCommand.payloadSize = 0;

  Command receivedCommand = {};

  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(true));
  EXPECT_CALL(*transport, receiveAck(_)) 
    .WillOnce(Invoke([=](const Command *command) -> bool { 
      memcpy((void*)command, &expectedCommand, sizeof(Command));
      return true; 
    }));
  EXPECT_CALL(*transport, sendCommand(_));

  underTest->processCommand(&commandHeartbeat, &receivedCommand);

  ASSERT_THAT(receivedCommand.type, Eq(CommandType::CONFIRM));
}