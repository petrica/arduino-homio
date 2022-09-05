#include <gmock/gmock.h>
#include <DeviceUnderTest.h>
#include <Matchers.h>

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

class DeviceTest : public Test
{
public:
    DeviceTransportMock *transport;
    DeviceUnderTest *underTest;

    void SetUp()
    {
        transport = new NiceMock<DeviceTransportMock>();
        underTest = new DeviceUnderTest(transport);
    }

    void TearDown()
    {
        delete underTest;
        underTest = nullptr;

        delete transport;
        transport = nullptr;
    }
};

TEST_F(DeviceTest, SendDatapointReturnTrue)
{
    Datapoint datapoint = {};
    datapoint.id = 1;
    datapoint.type = DatapointType::INTEGER;
    datapoint.value_int = 0;

    underTest->addDatapoint(&datapoint);
    ON_CALL(*transport, writeDatapoint)
        .WillByDefault(Return(true));

    ASSERT_TRUE(underTest->sendDatapoint(1));
}

TEST_F(DeviceTest, SendNonExistingDatapointReturnFalse)
{
    Datapoint datapoint = {};
    datapoint.id = 1;
    datapoint.type = DatapointType::INTEGER;
    datapoint.value_int = 0;

    underTest->addDatapoint(&datapoint);

    ASSERT_FALSE(underTest->sendDatapoint(2));
}

TEST_F(DeviceTest, SendDatapointWritesToTransport)
{
    Datapoint datapoint = {};
    datapoint.id = 1;
    datapoint.type = DatapointType::INTEGER;
    datapoint.value_int = 0;

    underTest->addDatapoint(&datapoint);

    EXPECT_CALL(*transport, writeDatapoint(Eq(&datapoint)));

    underTest->sendDatapoint(1);
}

TEST_F(DeviceTest, DatapointsListIsEmpty) {
  uint8_t listCount = underTest->getDatapointsCount();
  ASSERT_THAT(listCount, Eq(0));
}

TEST_F(DeviceTest, DatapointsListHasCountOfOne) {
  Datapoint datapoint;
  underTest->addDatapoint(&datapoint);
  uint8_t listCount = underTest->getDatapointsCount();
  ASSERT_THAT(listCount, Eq(1));
}

// TEST_F(DeviceTest, WhenDeviceInitializedCapabilitiesAreSetToDefault)
// {
//     DeviceCapabilities capabilities = underTest->getCapabilities();

//     ASSERT_THAT(capabilities, Field(&DeviceCapabilities::heartbeatInterval, Eq(HOMIO_DEVICE_CAPABILITIES_HEARTBEAT_INTERVAL)));
//     ASSERT_THAT(capabilities, Field(&DeviceCapabilities::canReceive, Eq(HOMIO_DEVICE_CAPABILITIES_CAN_RECEIVE)));
// }

// TEST_F(DatapointSerializerTest, FailToDeserializeNonexistingDatapoint) {
//     datapoint.id = 1;
//     datapoint.type = DatapointType::BOOLEAN;
//     datapoint.value_bool = true;

//     underTest->addDatapoint(&datapoint);
//     underTest->serializeDatapoint(1, buffer);

//     buffer[0] = 2;

//     Datapoint *expectedDatapoint = underTest->unserializeDatapoint(buffer);

//     ASSERT_THAT(expectedDatapoint, Eq(nullptr));
// }

// TEST_F(DatapointSerializerTest, SerailizerReturnsEmptyIfNoDatapointToSerialize) {
//   datapoint.id = 1;
//   datapoint.type = DatapointType::BOOLEAN;
//   datapoint.value_bool = true;

//   ASSERT_THAT(underTest->serializeDatapoint(1, buffer), Eq(0));
// }
