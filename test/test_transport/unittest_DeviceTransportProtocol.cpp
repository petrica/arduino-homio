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
        payload = new uint8_t[HOMIO_COMMAND_PAYLOAD_SIZE];
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
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
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
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
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
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
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

TEST_F(DeviceTransportProtocolTest, WhenInDataSentThenReturnToIdle) {
    underTest->setState(TransportState::DATA_SEND);

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(TransportState::IDLE));
}

TEST_F(DeviceTransportProtocolTest, WhenInDataSentThenSendDatapointReportCommand) {
    underTest->setHubReceiveAddress(2);
    
    Command commandInstance = {};
    uint8_t commandPayload[HOMIO_COMMAND_PAYLOAD_SIZE];
    commandInstance.payload = commandPayload;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;
    uint8_t size = serializeCommand(&expectedCommand, buffer);

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&expectedCommand));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&commandInstance));
    EXPECT_CALL(*radio, send(2, EqualToArray(buffer, size), size));

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenDataSentFailsThenCommandQueueIsNotDequeued) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    Command commandInstance = {};
    uint8_t commandPayload[HOMIO_COMMAND_PAYLOAD_SIZE];
    commandInstance.payload = commandPayload;

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&expectedCommand));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&commandInstance));
    ON_CALL(*radio, send)
        .WillByDefault(Return(0));
    EXPECT_CALL(*commandQueue, dequeue)
        .Times(0);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenDataSentAndNoAckTheQueueIsNotDequeued) {
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

TEST_F(DeviceTransportProtocolTest, WhenDataSentForDeviceThenCommandQueueIsDequeued) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::CONFIRM;
    serializeCommand(&expectedCommand, buffer);

    Command commandInstance = {};
    uint8_t commandPayload[HOMIO_COMMAND_PAYLOAD_SIZE];
    commandInstance.payload = commandPayload;

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&sentCommand));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&commandInstance));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
        }));
    EXPECT_CALL(*commandQueue, dequeue)
        .Times(1);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenDataSentAndGotBackDatapointDeliverThenCommandShouldBeDequeued) {
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

    Command commandInstance = {};
    uint8_t commandPayload[HOMIO_COMMAND_PAYLOAD_SIZE];
    commandInstance.payload = commandPayload;

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&sentCommand));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&commandInstance));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
        }));
    EXPECT_CALL(*commandQueue, dequeue)
        .Times(1);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenInDataSentAndQueueEmptyThenDontSend) {
    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(nullptr));
    EXPECT_CALL(*radio, send)
        .Times(0);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenInLockRequestModeAndNoCommandInstanceAvailableBecomeIdle) {
    underTest->setState(TransportState::LOCK_REQUEST);

    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(nullptr));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(TransportState::IDLE));
}

TEST_F(DeviceTransportProtocolTest, WhenDataSentAndGotBackConfirmedThenDatapointIdShouldBeZero) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::CONFIRM;
    serializeCommand(&expectedCommand, buffer);

    Command commandInstance = {};
    uint8_t commandPayload[HOMIO_COMMAND_PAYLOAD_SIZE];
    commandInstance.payload = commandPayload;

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&sentCommand));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&commandInstance));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getDatapointId(), Eq(0));
}

TEST_F(DeviceTransportProtocolTest, WhenDataSentAndGotBackDatapointDeliverThenDatapointIdShouldBeReturned) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::DATAPOINT_DELIVER;

    Datapoint expectedDatapoint = {};
    expectedDatapoint.id = 1;
    expectedDatapoint.type = DatapointType::BOOLEAN;
    expectedDatapoint.value_bool = true;
    expectedCommand.payloadSize = serializeDatapoint(&expectedDatapoint, payload);
    expectedCommand.payload = payload;
    serializeCommand(&expectedCommand, buffer);

    Command commandInstance = {};
    uint8_t commandPayload[HOMIO_COMMAND_PAYLOAD_SIZE];
    commandInstance.payload = commandPayload;

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&sentCommand));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&commandInstance));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getDatapointId(), Eq(expectedDatapoint.id));
}

TEST_F(DeviceTransportProtocolTest, WhenDataSentAndGotBackDatapointDeliverThenDatapointShouldBeReturned) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::DATAPOINT_DELIVER;

    Datapoint expectedDatapoint = {};
    expectedDatapoint.id = 1;
    expectedDatapoint.type = DatapointType::BOOLEAN;
    expectedDatapoint.value_bool = true;
    expectedCommand.payloadSize = serializeDatapoint(&expectedDatapoint, payload);
    expectedCommand.payload = payload;
    serializeCommand(&expectedCommand, buffer);

    Command commandInstance = {};
    uint8_t commandPayload[HOMIO_COMMAND_PAYLOAD_SIZE];
    commandInstance.payload = commandPayload;

    underTest->setState(TransportState::DATA_SEND);

    ON_CALL(*commandQueue, peek)
        .WillByDefault(Return(&sentCommand));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&commandInstance));
    ON_CALL(*radio, send)
        .WillByDefault(Return(1));
    ON_CALL(*radio, hasAckData)
        .WillByDefault(Return(true));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
        }));
    EXPECT_CALL(*commandPool, returnCommandInstance(Eq(&commandInstance)));

    underTest->tick();

    Datapoint datapoint;
    ASSERT_TRUE(underTest->readDatapoint(&datapoint));
    ASSERT_THAT(datapoint.id, Eq(expectedDatapoint.id));
    ASSERT_THAT(underTest->getReceivedCommand(), Eq(nullptr));
}

TEST_F(DeviceTransportProtocolTest, WhenInIdleStateThenChecIfDataAvailable) {
    underTest->setState(TransportState::IDLE);

    EXPECT_CALL(*radio, hasData);
    EXPECT_CALL(*commandPool, borrowCommandInstance);

    underTest->tick();
}

TEST_F(DeviceTransportProtocolTest, WhenInIdleAndReceivedConfirmCommandThenReturnInstanceToPool) {
    Command instance;

    Command command;
    command.type = CommandType::CONFIRM;
    command.fromAddress = 1;
    command.toAddress = 10;
    command.payloadSize = 0;
    serializeCommand(&command, buffer);

    underTest->setState(TransportState::IDLE);

    ON_CALL(*radio, hasData)
        .WillByDefault(Return(true));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&instance));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
        }));
    EXPECT_CALL(*commandPool, returnCommandInstance(Eq(&instance)));

    underTest->tick();    
}

TEST_F(DeviceTransportProtocolTest, WhenInIdleAndReceivedDeliverCommandThenDoNotReturnInstanceToPool) {
    Command instance;

    Command command;
    command.type = CommandType::DATAPOINT_DELIVER;
    command.fromAddress = 1;
    command.toAddress = 10;
    command.payloadSize = 0;
    serializeCommand(&command, buffer);

    underTest->setState(TransportState::IDLE);

    ON_CALL(*radio, hasData)
        .WillByDefault(Return(true));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&instance));
    EXPECT_CALL(*radio, readData(_))
        .WillOnce(Invoke([=](void *data) -> void
        { 
            memcpy(data, buffer, HOMIO_BUFFER_SIZE);
        }));
    EXPECT_CALL(*commandPool, returnCommandInstance)
        .Times(0);

    underTest->tick();    
}

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