#include <gmock/gmock.h>
#include <TransportMock.h>
#include <DeviceUnderTest.h>
#include <matchers.h>

using namespace ::testing;
using namespace Homio;

// what do I need my test to do first? I have a device that I would like to do something
// what does a divce need to do, send datapoints and react to events received from hub, the simplest test would be to add a datapoint? No
// do I need to get the list of the datapoints? yes
// So what would be the real use of my class, add a datapoint and send the data to the hub
// serialize datapoint to binary
// communication protocol could also be implemented, what are the rules of the protocol?
// the protocol should send commands in the queue, why not keep the actual buffer on the queue and not commands?
// 1. if there is nothing to send, do nothing
// 2. if there is binary data pending in the queue, start the sending process
// 3. send a lock request
// 4. read the ack payload and if we got a lock for our device go to next step
// 5. send the data to hub, don't care about the payload
// 6. confirm the hub ack the data that we have sent
// other functions
// unserialize the response command - these are functions that should be determined based on a bahavior
// serialize the response command
// simplesting thing: when adding binary data to buffer, the buffer should not be empty

// classes
// Transport
  // serialize data send
  // unserialise data receive
// NRFLiteTransport
// RF24Transport
// Device insert NRFiteTransport

class DeviceTest : public Test {
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

TEST_F(DeviceTest, CommandQueueHasSizeOfOne) {
  Command command;
  underTest->enqueueCommand(&command);
  uint8_t queueSize = underTest->getCommandQueueSize();
  ASSERT_THAT(queueSize, Eq(1));
}

TEST_F(DeviceTest, CommandQueueIsEmpty) {
  uint8_t queueSize = underTest->getCommandQueueSize();
  ASSERT_THAT(queueSize, Eq(0));
}

TEST_F(DeviceTest, DequeueReportCommand) {
  Command command = {};
  command.type = CommandType::DATAPOINT_REPORT;
  underTest->enqueueCommand(&command);
  Command *actual;
  actual = underTest->dequeueCommand();
  ASSERT_THAT(actual->type, Eq(CommandType::DATAPOINT_REPORT));
}

TEST_F(DeviceTest, DequeueEmptyQueueRemainsEmpty) {
  underTest->dequeueCommand();
  uint8_t queueSize = underTest->getCommandQueueSize();
  ASSERT_THAT(queueSize, Eq(0));
}

// TEST_F(DeviceTest, EnqueueMoreThanMaxReturnsFalse)

// TEST_F(DeviceTest, DatapointEnqueueReportCommand) {
//   Datapoint datapoint = {};
//   datapoint.id = 1;
//   datapoint.type = DatapointType::INTEGER;
//   datapoint.value_int = 0;
//   underTest->addDatapoint(&datapoint);
//   underTest->reportDatapoint(Datapoint *datapoint);


//   ASSERT_THAT()
// }


