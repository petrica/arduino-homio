#include <gmock/gmock.h>
#include <TransportMock.h>
#include <DeviceUnderTest.h>
#include <matchers.h>

using namespace ::testing;
using namespace Homio;

class ProtocolTest : public Test {
  public:
    DeviceUnderTest *underTest;
    Transport *transport;

  void SetUp() {
    transport = new TransportMock();
    underTest = new DeviceUnderTest(transport);
  }

  void TearDown() {
    delete underTest;
    underTest = nullptr;

    delete transport;
    transport = nullptr;
  }
};

TEST_F(ProtocolTest, DeviceStateDefaultToIdle) {
  ASSERT_THAT(underTest->getState(), Eq(DeviceState::IDLE));
}

// Implement the protocol
TEST_F(ProtocolTest, ProtocolDoNothingIfNothingToSend) {
  underTest->setState(DeviceState::IDLE);
  underTest->tick();
  ASSERT_THAT(underTest->getState(), Eq(DeviceState::IDLE));
}

TEST_F(ProtocolTest, ProtocolRequestLockStateIfSomethingToSend) {
  Command command = {};
  command.type = CommandType::HEARTBEAT;
  command.fromAddress = 10;
  command.toAddress = 1;
  command.payloadSize = 0;
  underTest->setState(DeviceState::IDLE);
  underTest->enqueueCommand(&command);
  underTest->tick();
  ASSERT_THAT(underTest->getState(), Eq(DeviceState::LOCK_REQUEST));
}

TEST_F(ProtocolTest, ProtocolRequestLockReturnEmptyAck) {
  Command command = {};
  command.type = CommandType::HEARTBEAT;
  command.fromAddress = 10;
  command.toAddress = 1;
  command.payloadSize = 0;
  underTest->setState(DeviceState::IDLE);
  underTest->enqueueCommand(&command);
  // Lock requested and return empty ack
  underTest->tick();
}