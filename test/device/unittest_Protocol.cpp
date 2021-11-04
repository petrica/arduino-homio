#include <gmock/gmock.h>
#include <DeviceUnderTest.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class ProtocolTest : public Test
{
public:
    DeviceUnderTest *underTest;
    ProtocolMock *protocol;
    CommandPool *commandPool;

    void SetUp()
    {
        protocol = new StrictMock<ProtocolMock>();
        commandPool = new CommandPoolMock();
        underTest = new DeviceUnderTest(protocol, commandPool);
    }

    void TearDown()
    {
        delete underTest;
        underTest = nullptr;

        delete protocol;
        protocol = nullptr;
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

    ON_CALL(*protocol, processCommand)
        .WillByDefault(Return(true));
    underTest->setState(DeviceState::LOCK_REQUEST);
    EXPECT_CALL(*protocol, processCommand(
                               Field(&Command::type, Eq(CommandType::LOCK_REQUEST)), _));

    underTest->tick();
}

TEST_F(ProtocolTest, WhenInLockRequestAndReceiveEmptyResponseStatusShouldReturnToIdle)
{
    Command expectedCommand = {};
    underTest->setState(DeviceState::LOCK_REQUEST);

    ON_CALL(*protocol, processCommand)
        .WillByDefault(Return(false));
    EXPECT_CALL(*protocol, processCommand(_, _))
        .WillOnce(Invoke([=](const Command *command, Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return false; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::IDLE));
}

TEST_F(ProtocolTest, WhenInLockRequestAndReceiveLockForOtherDeviceThenStatusIdDelay) {
    Command expectedCommand = {};
    expectedCommand.type = CommandType::LOCK_DELIVER;
    expectedCommand.fromAddress = 1;
    expectedCommand.toAddress = 11;
    expectedCommand.payloadSize = 0;

    underTest->setState(DeviceState::LOCK_REQUEST);
    
    ON_CALL(*protocol, processCommand)
        .WillByDefault(Return(false));
    EXPECT_CALL(*protocol, processCommand(_, _))
        .WillOnce(Invoke([=](const Command *command, Command *receivedCommand) -> bool
        { 
            memcpy((void*)receivedCommand, &expectedCommand, sizeof(Command));
            return false; 
        }));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::LOCK_DELAY));
}

TEST_F(ProtocolTest, WhenInLockRequestAndReceivedLockRequestThenStatusIsDataSend) {
    underTest->setState(DeviceState::LOCK_REQUEST);

    ON_CALL(*protocol, processCommand)
        .WillByDefault(Return(true));
    EXPECT_CALL(*protocol, processCommand(_, _));

    underTest->tick();

    ASSERT_THAT(underTest->getState(), Eq(DeviceState::DATA_SEND));
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

    ON_CALL(*protocol, processCommand)
        .WillByDefault(Return(true));
    EXPECT_CALL(*protocol, processCommand(
        Field(&Command::type, Eq(CommandType::DATAPOINT_REPORT)), _)
    );

    underTest->tick();
}

TEST_F(ProtocolTest, WhenDataSendThenCommandQueueIsEmpty) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->enqueueCommand(&expectedCommand);

    ON_CALL(*protocol, processCommand)
        .WillByDefault(Return(true));
    EXPECT_CALL(*protocol, processCommand);

    underTest->tick();

    ASSERT_THAT(underTest->getCommandQueueSize(), Eq(0));  
}

TEST_F(ProtocolTest, WhenDataSendFailsThenCommandQueueIsNotEmpty) {
    Command expectedCommand = {};
    expectedCommand.fromAddress = 10;
    expectedCommand.toAddress = 1;
    expectedCommand.type = CommandType::DATAPOINT_REPORT;

    underTest->setState(DeviceState::DATA_SEND);
    underTest->enqueueCommand(&expectedCommand);

    ON_CALL(*protocol, processCommand)
        .WillByDefault(Return(false));
    EXPECT_CALL(*protocol, processCommand);

    underTest->tick();

    ASSERT_THAT(underTest->getCommandQueueSize(), Gt(0));   
}

TEST_F(ProtocolTest, WhenInDataSendAndQueueEmptyThenDontSend) {
    underTest->setState(DeviceState::DATA_SEND);
    
    EXPECT_CALL(*protocol, processCommand)
        .Times(0);

    underTest->tick();
}