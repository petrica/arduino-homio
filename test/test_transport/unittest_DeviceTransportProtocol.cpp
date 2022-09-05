#include <gmock/gmock.h>
#include <DeviceTransportUnderTest.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class DeviceTransportProtocolTest : public Test
{
public:
    DeviceTransportUnderTest *underTest;
    CommandPoolMock *commandPool;
    CommandQueueMock *commandQueue;
    NRFLiteMock *radio;
    uint8_t deviceAddress = 10;
    uint8_t hubAddress = 1;
    uint8_t *payload;
    uint8_t *buffer;

    void SetUp()
    {
        radio = new NiceMock<NRFLiteMock>();
        commandPool = new CommandPoolMock();
        commandQueue = new NiceMock<CommandQueueMock>();
        underTest = new DeviceTransportUnderTest(deviceAddress, hubAddress, radio, commandPool, commandQueue);
        payload = new uint8_t[10];
        buffer = new uint8_t[HOMIO_BUFFER_SIZE];
    }

    void TearDown()
    {
        delete underTest;
        underTest = nullptr;

        delete radio;
        radio = nullptr;

        delete commandPool;
        commandPool = nullptr;

        delete commandQueue;
        commandQueue = nullptr;

        delete[] payload;
        payload = nullptr;

        delete[] buffer;
        buffer = nullptr;
    }
};

TEST_F(DeviceTransportProtocolTest, TransportStateDefaultToIdle)
{
    ASSERT_THAT(underTest->getState(), Eq(TransportState::IDLE));
}

TEST_F(DeviceTransportProtocolTest, DoNothingIfNothingToSend)
{
    underTest->setState(TransportState::IDLE);
    underTest->tick();
    ASSERT_THAT(underTest->getState(), Eq(TransportState::IDLE));
}

TEST_F(DeviceTransportProtocolTest, WhenElementsInQueueSendStatusToLockRequest)
{
    Command command = {};
    underTest->setState(TransportState::IDLE);
    ON_CALL(*commandQueue, getSize)
        .WillByDefault(Return(1));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(TransportState::LOCK_REQUEST));
}

TEST_F(DeviceTransportProtocolTest, WhenInLockRequestStateSendLockRequestCommand)
{
    Command actual = {};
    Command expected = {};
    expected.type = CommandType::LOCK_REQUEST;
    expected.fromAddress = deviceAddress;
    expected.toAddress = hubAddress;
    uint8_t size = serializeCommand(&expected, buffer);

    underTest->setState(TransportState::LOCK_REQUEST);
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    EXPECT_CALL(*radio, send(hubAddress, EqualToArray(buffer, size), size));

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenInLockRequestAndReceiveEmptyResponseStatusShouldReturnToIdle)
{
    Command expectedCommand = {};
    Command actual = {};
    underTest->setState(TransportState::LOCK_REQUEST);

    ON_CALL(*radio, send)
        .WillByDefault(Return(true));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(false));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(TransportState::IDLE));
}

TEST_F(DeviceTransportProtocolTest, WhenInLockRequestAndReceiveLockForOtherDeviceThenStatusIdDelay) {
    Command actual = {};
    Command expectedCommand = {};
    expectedCommand.type = CommandType::LOCK_DELIVER;
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 11;
    expectedCommand.payloadSize = 1;
    expectedCommand.payload = payload;
    serializeCommand(&expectedCommand, buffer);

    underTest->setState(TransportState::LOCK_REQUEST);
    
    ON_CALL(*radio, send)
        .WillByDefault(Return(true));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, send);
    EXPECT_CALL(*radio, hasAckData);
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy((void*)data, buffer, HOMIO_BUFFER_SIZE);
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(TransportState::LOCK_DELAY));
}

TEST_F(DeviceTransportProtocolTest, WhenInLockRequestAndReceivedLockRequestThenStatusIsDataSend) {
    Command actual = {};
    actual.payload = payload;

    uint8_t expectedPayload[1] = { 2 }; 
    Command expectedCommand = {};
    expectedCommand.type = CommandType::LOCK_DELIVER;
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.payloadSize = 1;
    expectedCommand.payload = expectedPayload;
    serializeCommand(&expectedCommand, buffer);

    underTest->setState(TransportState::LOCK_REQUEST);

    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    ON_CALL(*radio, send)
        .WillByDefault(Return(true));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy((void*)data, buffer, HOMIO_BUFFER_SIZE);
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(TransportState::DATA_SEND));
}

TEST_F(DeviceTransportProtocolTest, WhenInLockRequestAndReceivedLockRequestThenHubReceiveAddressShouldBePopulated) {
    Command actual = {};
    actual.payload = payload;

    uint8_t expectedPayload[1] = { 2 }; 
    Command expectedCommand = {};
    expectedCommand.type = CommandType::LOCK_DELIVER;
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.payloadSize = 1;
    expectedCommand.payload = expectedPayload;

    underTest->setState(TransportState::LOCK_REQUEST);

    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    ON_CALL(*radio, send)
        .WillByDefault(Return(true));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy((void*)data, buffer, HOMIO_BUFFER_SIZE);
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getHubReceiveAddress(), Eq(2));
}

TEST_F(DeviceTransportProtocolTest, WhenInLockDelayThenDelay) {
    underTest->setState(TransportState::LOCK_DELAY);

    ArduinoMock *arduinoInstance = arduinoMockInstance();
    EXPECT_CALL(*arduinoInstance, delay(Eq(HOMIO_TRANSPORT_CONFLICT_DELAY)));

    underTest->tick();

    releaseArduinoMock();
}

TEST_F(DeviceTransportProtocolTest, WhenInDataSendThenReturnToIdle) {
    underTest->setState(TransportState::DATA_SEND);

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(TransportState::IDLE));
}

TEST_F(DeviceTransportProtocolTest, WhenInDataSendThenSendDatapointReportCommand) {
    underTest->setHubReceiveAddress(2);

    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;
    uint8_t size = serializeCommand(&expectedCommand, buffer);

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&expectedCommand));
    EXPECT_CALL(*radio, send(2, EqualToArray(buffer, size), size));

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenDataSendFailsThenCommandQueueIsNotDequeued) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&expectedCommand));
    ON_CALL(*radio, send)
        .WillByDefault(Return(0));
    EXPECT_CALL(*commandQueue, dequeue)
        .Times(0);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenDataSendAndNoAckTheQueueIsNotDequeued) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&expectedCommand));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(false));
    EXPECT_CALL(*commandQueue, dequeue)
        .Times(0);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenDataSendForDeviceThenCommandQueueIsDequeued) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::CONFIRM;
    serializeCommand(&expectedCommand, buffer);

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&sentCommand));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy((void*)data, buffer, HOMIO_BUFFER_SIZE);
        }));
    EXPECT_CALL(*commandQueue, dequeue)
        .Times(1);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenDataSendIsNotConfirmedThenCommandQueueIsNotDequeued) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::DATAPOINT_DELIVER;
    serializeCommand(&expectedCommand, buffer);

    underTest->setState(TransportState::DATA_SEND);
   

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&sentCommand));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy((void*)data, buffer, HOMIO_BUFFER_SIZE);
        }));
    EXPECT_CALL(*commandQueue, dequeue)
        .Times(0);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenInDataSendAndQueueEmptyThenDontSend) {
    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(nullptr));
    EXPECT_CALL(*radio, send)
        .Times(0);

    underTest->tick();
}

// TEST_F(DeviceTest, UnsuccessfulSendDatapointIfNoMoreObjectsInPool)
// {
//     Datapoint datapoint = {};
//     datapoint.id = 1;
//     datapoint.type = DatapointType::INTEGER;
//     datapoint.value_int = 0;

//     underTest->addDatapoint(&datapoint);
//     ON_CALL(*commandPool, borrowCommandInstance)
//         .WillByDefault(Return(nullptr));
//     EXPECT_CALL(*commandPool, borrowCommandInstance());

//     ASSERT_FALSE(underTest->sendDatapoint(1));
// }

// TEST_F(DeviceTest, WhenSendingDatapointFromAndToAddressesShouldBePopulated) {
//     Datapoint datapoint = {};
//     datapoint.id = 1;
//     datapoint.type = DatapointType::INTEGER;
//     datapoint.value_int = 0;

//     underTest->addDatapoint(&datapoint);
//     ON_CALL(*commandPool, borrowCommandInstance)
//         .WillByDefault(Return(command));
//     EXPECT_CALL(*commandPool, borrowCommandInstance());
    
//     underTest->sendDatapoint(1);

//     Command *actual = underTest->peekCommand();

//     ASSERT_THAT(actual, Field(&Command::fromAddress, Eq(10)));
//     ASSERT_THAT(actual, Field(&Command::toAddress, Eq(1)));
// }