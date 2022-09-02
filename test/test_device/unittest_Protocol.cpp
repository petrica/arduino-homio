#include <gmock/gmock.h>
#include <DeviceUnderTest.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class ProtocolTest : public Test
{
public:
    DeviceUnderTest *underTest;
    TransportMock *transport;
    CommandPoolMock *commandPool;
    uint8_t *payload;

    void SetUp()
    {
        transport = new StrictMock<TransportMock>();
        commandPool = new CommandPoolMock();
        underTest = new DeviceUnderTest(transport, commandPool);
        payload = new uint8_t[10];
    }

    void TearDown()
    {
        delete underTest;
        underTest = nullptr;

        delete transport;
        transport = nullptr;

        delete[] payload;
        payload = nullptr;

        delete commandPool;
        commandPool = nullptr;
    }
};

TEST_F(ProtocolTest, DeviceStateDefaultToIdle)
{
    ASSERT_THAT(underTest->getState(), Eq(DeviceState::IDLE));
}

TEST_F(ProtocolTest, DoNothingIfNothingToSend)
{
    underTest->setState(DeviceState::IDLE);
    underTest->tick();
    ASSERT_THAT(underTest->getState(), Eq(DeviceState::IDLE));
}

TEST_F(ProtocolTest, WhenElementsInQueueSendStatusToLockRequest)
{
    Command command = {};
    underTest->setState(DeviceState::IDLE);
    underTest->enqueueCommand(&command);

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::LOCK_REQUEST));
}

TEST_F(ProtocolTest, WhenInLockRequestStateSendLockRequestCommand)
{
    Command actual = {};

    underTest->setState(DeviceState::LOCK_REQUEST);
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    EXPECT_CALL(*transport, sendCommand(
                               Field(&Command::type, Eq(CommandType::LOCK_REQUEST))));

    underTest->tick();
}

TEST_F(ProtocolTest, WhenInLockRequestAndReceiveEmptyResponseStatusShouldReturnToIdle)
{
    Command expectedCommand = {};
    Command actual = {};
    underTest->setState(DeviceState::LOCK_REQUEST);

    ON_CALL(*transport, sendCommand(_))
        .WillByDefault(Return(true));
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    EXPECT_CALL(*transport, sendCommand(_));
    EXPECT_CALL(*transport, receiveAck(_))
        .WillOnce(Invoke([=](Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return true; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::IDLE));
}

TEST_F(ProtocolTest, WhenInLockRequestAndReceiveLockForOtherDeviceThenStatusIdDelay) {
    Command actual = {};
    Command expectedCommand = {};
    expectedCommand.type = CommandType::LOCK_DELIVER;
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 11;
    expectedCommand.payloadSize = 1;
    expectedCommand.payload = payload;

    underTest->setState(DeviceState::LOCK_REQUEST);
    
    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    ON_CALL(*transport, sendCommand(_))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(_));
    EXPECT_CALL(*transport, receiveAck(_))
        .WillOnce(Invoke([=](Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return true; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::LOCK_DELAY));
}

TEST_F(ProtocolTest, WhenInLockRequestAndReceivedLockRequestThenStatusIsDataSend) {
    Command actual = {};
    Command expectedCommand = {};
    expectedCommand.type = CommandType::LOCK_DELIVER;
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.payloadSize = 1;
    expectedCommand.payload = payload;

    underTest->setState(DeviceState::LOCK_REQUEST);

    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    ON_CALL(*transport, sendCommand(_))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(_));
    EXPECT_CALL(*transport, receiveAck(_))
        .WillOnce(Invoke([=](Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return true; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::DATA_SEND));
}

TEST_F(ProtocolTest, WhenInLockRequestAndReceivedLockRequestThenHubReceiveAddressShouldBePopulated) {
    Command actual = {};
    Command expectedCommand = {};
    expectedCommand.type = CommandType::LOCK_DELIVER;
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.payloadSize = 1;
    payload[0] = 2;
    expectedCommand.payload = payload;

    underTest->setState(DeviceState::LOCK_REQUEST);

    ON_CALL(*commandPool, borrowCommandInstance)
        .WillByDefault(Return(&actual));
    EXPECT_CALL(*commandPool, borrowCommandInstance);
    EXPECT_CALL(*commandPool, returnCommandInstance);
    ON_CALL(*transport, sendCommand(_))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(_));
    EXPECT_CALL(*transport, receiveAck(_))
        .WillOnce(Invoke([=](Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return true; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getHubReceiveAddress(), Eq(2));
}

TEST_F(ProtocolTest, WhenInLockDelayThenDelay) {
    underTest->setState(DeviceState::LOCK_DELAY);

    ArduinoMock *arduinoInstance = arduinoMockInstance();
    EXPECT_CALL(*arduinoInstance, delay(Eq(100)));

    underTest->tick();

    releaseArduinoMock();
}

TEST_F(ProtocolTest, WhenInDataSendThenReturnToIdle) {
    underTest->setState(DeviceState::DATA_SEND);

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::IDLE));
}

TEST_F(ProtocolTest, WhenInDataSendThenSendDatapointReportCommand) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->enqueueCommand(&expectedCommand);

    ON_CALL(*transport, sendCommand(_, _))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(
        Field(&Command::type, Eq(CommandType::DATAPOINT_REPORT)), _)
    );
    EXPECT_CALL(*transport, receiveAck);

    underTest->tick();
}

TEST_F(ProtocolTest, WhenDataSendFailsThenCommandQueueIsNotEmpty) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->enqueueCommand(&expectedCommand);

    ON_CALL(*transport, sendCommand(_, _))
        .WillByDefault(Return(false));
    EXPECT_CALL(*transport, sendCommand(_, _));

    underTest->tick();

    ASSERT_THAT(underTest->getCommandQueueSize(), Gt(0));
}

TEST_F(ProtocolTest, WhenDataSendAndNoAckTheQueueIsNotEmpty) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->enqueueCommand(&expectedCommand);

    ON_CALL(*transport, sendCommand(_, _))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(_, _));
    ON_CALL(*transport, receiveAck)
        .WillByDefault(Return(false));
    EXPECT_CALL(*transport, receiveAck);

    underTest->tick();

    ASSERT_THAT(underTest->getCommandQueueSize(), Gt(0));
}

TEST_F(ProtocolTest, WhenDataSendForDeviceThenCommandQueueIsEmpty) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::CONFIRM;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->enqueueCommand(&sentCommand);

    ON_CALL(*transport, sendCommand(_, _))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(_, _));
    EXPECT_CALL(*transport, receiveAck(_))
        .WillOnce(Invoke([=](Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return true; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getCommandQueueSize(), Eq(0));
}

TEST_F(ProtocolTest, WhenDataSendIsNotConfirmedThenCommandQueueIsNotEmpty) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    Command expectedCommand = {};
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 10;
    expectedCommand.type = CommandType::DATAPOINT_DELIVER;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->enqueueCommand(&sentCommand);

    ON_CALL(*transport, sendCommand(_, _))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(_, _));
    EXPECT_CALL(*transport, receiveAck(_))
        .WillOnce(Invoke([=](Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return true; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getCommandQueueSize(), Gt(0));
}

TEST_F(ProtocolTest, WhenDataSendThenCallWithHubReceivedAddress) {
    Command sentCommand = {};
    sentCommand.fromAddress = 10;
    sentCommand.toAddress = 1;
    sentCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->setHubReceiveAddress(2);
    underTest->enqueueCommand(&sentCommand);

    ON_CALL(*transport, sendCommand(_, _))
        .WillByDefault(Return(true));
    EXPECT_CALL(*transport, sendCommand(_, Eq(2)));
    EXPECT_CALL(*transport, receiveAck(_));

    underTest->tick();
}

TEST_F(ProtocolTest, WhenInDataSendAndQueueEmptyThenDontSend) {
    underTest->setState(DeviceState::DATA_SEND);
    
    EXPECT_CALL(*transport, sendCommand(_))
        .Times(0);

    underTest->tick();
}