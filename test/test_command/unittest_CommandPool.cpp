#include <gmock/gmock.h>
#include <CommandPool.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class Homio::CommandPoolUnderTest: public CommandPool {
    public:
        uint8_t getPoolCount() {
            return CommandPool::poolCount_;
        }
};

class CommandPoolTest : public Test {
    public:
        CommandPoolUnderTest *underTest;

    void SetUp() {
        underTest = new CommandPoolUnderTest();
    }

    void TearDown() {
        delete underTest;
        underTest = nullptr;
    }
};

TEST_F(CommandPoolTest, GetObjectInstanceWhenRequested) {
    Command *command = underTest->borrowCommandInstance();

    ASSERT_THAT(command->type, Eq(CommandType::HEARTBEAT));
}

TEST_F(CommandPoolTest, AvailableObjectsEqualsPoolSize) {
    ASSERT_THAT(underTest->getPoolCount(), Eq(HOMIO_COMMAND_POOL_SIZE));
}

TEST_F(CommandPoolTest, AvailableObjectsMissingOneWhenInstanceRequested) {
    underTest->borrowCommandInstance();

    ASSERT_THAT(underTest->getPoolCount(), Eq(HOMIO_COMMAND_POOL_SIZE - 1));
}

TEST_F(CommandPoolTest, AvailableObjectsEqualsPoolSizeWhenPuttingBackObject) {
    Command *command = underTest->borrowCommandInstance();

    underTest->returnCommandInstance(command);

    ASSERT_THAT(underTest->getPoolCount(), Eq(HOMIO_COMMAND_POOL_SIZE));
}

TEST_F(CommandPoolTest, GetBackSameInstanceAfterPuttingItBack) {
    Command *actual = underTest->borrowCommandInstance();
    actual->type = CommandType::DATAPOINT_REPORT;

    underTest->returnCommandInstance(actual);

    ASSERT_THAT(underTest->borrowCommandInstance(), Eq(actual));
}

TEST_F(CommandPoolTest, GetBackSameInstancesAfterPuttingThemBack) {
    Command *actual = underTest->borrowCommandInstance();
    Command *other = underTest->borrowCommandInstance();
    underTest->returnCommandInstance(other);
    underTest->returnCommandInstance(actual);

    ASSERT_THAT(underTest->borrowCommandInstance(), Eq(actual));
}

TEST_F(CommandPoolTest, WhenRequestingTheEntirePoolThePoolIsEmpty) {
    Command *actual;
    for(uint8_t i = 0; i <= HOMIO_COMMAND_POOL_SIZE; i++) {
        actual = underTest->borrowCommandInstance();
    }

    ASSERT_THAT(underTest->getPoolCount(), Eq(0));
}

TEST_F(CommandPoolTest, PuttingBackObjectWhenPoolIsFullShouldDoNothing) {
    Command command;

    underTest->returnCommandInstance(&command);

    ASSERT_THAT(underTest->getPoolCount(), Eq(HOMIO_COMMAND_POOL_SIZE));
}

TEST_F(CommandPoolTest, FillingCommandPayloadWithDataShouldRemainTheSame) {
    Command *command;
    command = underTest->borrowCommandInstance();
    uint8_t payloadSize = HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE;
    uint8_t actual[HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE];
    for(uint i =0; i < payloadSize;  i++) {
        actual[i] = i;
    }
    memcpy(command->payload, actual, sizeof(actual));

    underTest->returnCommandInstance(command);

    command = underTest->borrowCommandInstance();

    ASSERT_THAT(command->payload, EqualToArray(actual, payloadSize));
}