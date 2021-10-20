#include <gmock/gmock.h>
#include <TransportMock.h>
#include <device.h>
#include <matchers.h>

using namespace ::testing;
using namespace Homio;

class ProtocolTest : public Test {
  public:
    Device *device;
    Transport *transport;

  void SetUp() {
    transport = new TransportMock();
    device = new Device(transport);
  }

  void TearDown() {
    delete device;
    device = nullptr;

    delete transport;
    transport = nullptr;
  }
};

TEST_F(ProtocolTest, DeviceStateDefaultToIdle) {
  ASSERT_THAT(device->getState(), Eq(DeviceState::IDLE));
}

// Implement the protocol
TEST_F(ProtocolTest, ProtocolDoNothingIfNothingToSend) {
  device->setState(DeviceState::IDLE);
  device->tick();
  ASSERT_THAT(device->getState(), Eq(DeviceState::IDLE));
}

TEST_F(ProtocolTest, ProtocolRequestLockStateIfSomethingToSend) {
  Command command = {};
  command.type = CommandType::HEARTBEAT;
  command.fromAddress = 10;
  command.toAddress = 1;
  command.payloadSize = 0;
  device->setState(DeviceState::IDLE);
  device->enqueueCommand(&command);
  device->tick();
  ASSERT_THAT(device->getState(), Eq(DeviceState::LOCK_REQUEST));
}

TEST_F(ProtocolTest, ProtocolRequestLockReturnEmptyAck) {
  Command command = {};
  command.type = CommandType::HEARTBEAT;
  command.fromAddress = 10;
  command.toAddress = 1;
  command.payloadSize = 0;
  device->setState(DeviceState::IDLE);
  device->enqueueCommand(&command);
  // Lock requested and return empty ack
  device->tick();
}