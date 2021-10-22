#include <gmock/gmock.h>
#include <DeviceUnderTest.h>
#include <TransportMock.h>
#include <matchers.h>

using namespace ::testing;
using namespace Homio;

class DeviceProcessCommand : public Test {
  public:
    DeviceUnderTest *underTest;
    TransportMock *transport;
    Command commandHeartbeat = {};

  void SetUp() {
    transport = new TransportMock();
    underTest = new DeviceUnderTest(transport);

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

TEST_F(DeviceProcessCommand, SuccessfullyProcessIfAllConditionsMet) {
  Command expectedCommand = {};
  expectedCommand.type = CommandType::CONFIRM;
  expectedCommand.fromAddress = 1;
  expectedCommand.toAddress = 10;
  expectedCommand.payloadSize = 0;

  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(true));
  EXPECT_CALL(*transport, receiveAck(_)) 
    .WillOnce(Invoke([=](const Command *command) -> bool { 
      memcpy((void*)command, &expectedCommand, sizeof(Command));
      return true; 
    }));
  EXPECT_CALL(*transport, sendCommand(_));

  ASSERT_TRUE(underTest->processCommand(&commandHeartbeat));
}

TEST_F(DeviceProcessCommand, FailProcessIfNoAckData) {
  
  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(true));
  ON_CALL(*transport, receiveAck)
    .WillByDefault(Return(false));

  EXPECT_CALL(*transport, sendCommand(_));
  EXPECT_CALL(*transport, receiveAck(_));

  ASSERT_FALSE(underTest->processCommand(&commandHeartbeat));
}

TEST_F(DeviceProcessCommand, FailProcessIfSendFailed) {
  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(false));

  EXPECT_CALL(*transport, sendCommand(_));
  EXPECT_CALL(*transport, receiveAck(_))
    .Times(0);

  ASSERT_FALSE(underTest->processCommand(&commandHeartbeat));
}

TEST_F(DeviceProcessCommand, FailProcessIfAckDataIntendedForOtherDevice) {
  Command expectedCommand = {};
  expectedCommand.type = CommandType::CONFIRM;
  expectedCommand.fromAddress = 1;
  expectedCommand.toAddress = 11;
  expectedCommand.payloadSize = 0;

  ON_CALL(*transport, sendCommand)
    .WillByDefault(Return(true));
  EXPECT_CALL(*transport, receiveAck(_)) 
    .WillOnce(Invoke([=](const Command *command) -> bool { 
      memcpy((void*)command, &expectedCommand, sizeof(Command));
      return true; 
    }));
  EXPECT_CALL(*transport, sendCommand(_));

  ASSERT_FALSE(underTest->processCommand(&commandHeartbeat));
}

