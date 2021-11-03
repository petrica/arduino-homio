#include <gmock/gmock.h>
#include <DeviceUnderTest.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class DeviceCommandQueueTest : public Test {
  public:
    DeviceUnderTest *underTest;
    ProtocolMock *protocol;
    CommandPoolMock *commandPool;

  void SetUp() {
    protocol = new ProtocolMock();
    commandPool = new CommandPoolMock();
    underTest = new DeviceUnderTest(protocol, commandPool);
  }

  void TearDown() {
    delete underTest;
    underTest = nullptr;

    delete protocol;
    protocol = nullptr;

    delete commandPool;
    commandPool = nullptr;
  }
};

TEST_F(DeviceCommandQueueTest, EnqueuingFirstCommandReturnsTrue) {
  Command command;
  bool actual = underTest->enqueueCommand(&command);
  ASSERT_TRUE(actual);
}

TEST_F(DeviceCommandQueueTest, CommandQueueHasSizeOfOne) {
  Command command;
  underTest->enqueueCommand(&command);
  uint8_t queueSize = underTest->getCommandQueueSize();
  ASSERT_THAT(queueSize, Eq(1));
}

TEST_F(DeviceCommandQueueTest, WhenStartedCommandQueueIsEmpty) {
  uint8_t queueSize = underTest->getCommandQueueSize();
  ASSERT_THAT(queueSize, Eq(0));
}

TEST_F(DeviceCommandQueueTest, WhenEnqueuingAndDequeuingShouldGiveTheSameCommand) {
  Command command = {};
  underTest->enqueueCommand(&command);
  Command *actual;
  actual = underTest->dequeueCommand();
  ASSERT_THAT(actual, Eq(&command));
}

TEST_F(DeviceCommandQueueTest, WhenDequeueEmptyQueueRemainsEmpty) {
  underTest->dequeueCommand();
  uint8_t queueSize = underTest->getCommandQueueSize();
  ASSERT_THAT(queueSize, Eq(0));
}

TEST_F(DeviceCommandQueueTest, WhenEnqueueingMoreItemsFirstDequeuedIsTheFirstQueued) {
  Command commandFirst = {};
  Command commandSecond = {};
  underTest->enqueueCommand(&commandFirst);
  underTest->enqueueCommand(&commandSecond);
  Command *actual;
  actual = underTest->dequeueCommand();
  ASSERT_THAT(actual, Eq(&commandFirst));
}

TEST_F(DeviceCommandQueueTest, WhenEnqueueingMoreItemsSecondDequeuedIsTheSecondQueued) {
  Command commandFirst = {};
  Command commandSecond = {};
  underTest->enqueueCommand(&commandFirst);
  underTest->enqueueCommand(&commandSecond);
  Command *actual;
  underTest->dequeueCommand();
  actual = underTest->dequeueCommand();
  ASSERT_THAT(actual, Eq(&commandSecond));
}

TEST_F(DeviceCommandQueueTest, WhenEnqueuingAndTheQueueIfFullReturnFalse) {
  Command command = {};
  for(uint8_t i = 0; i < HOMIO_COMMAND_QUEUE_SIZE; i++) {
    underTest->enqueueCommand(&command);  
  }
  
  ASSERT_FALSE(underTest->enqueueCommand(&command));
}

TEST_F(DeviceCommandQueueTest, WhenEnqueuingMoreThanTheQueueSizeQueueStillReturnsTheExepctedObject) {
  Command command = {};
  for(uint8_t i = 0; i < HOMIO_COMMAND_QUEUE_SIZE; i++) {
    underTest->enqueueCommand(&command);  
  }
  underTest->dequeueCommand();
  Command actual = {};
  underTest->enqueueCommand(&actual);
  for(uint8_t i = 0; i < HOMIO_COMMAND_QUEUE_SIZE - 1; i++) {
    underTest->dequeueCommand();  
  }

  ASSERT_THAT(underTest->dequeueCommand(), Eq(&actual));
}

TEST_F(DeviceCommandQueueTest, WhenPeekTheQueueTheSizeRemainsTheSame) {
  Command command = {};
  underTest->enqueueCommand(&command);
  underTest->peekCommand();
  ASSERT_THAT(underTest->getCommandQueueSize(), Eq(1));
}

TEST_F(DeviceCommandQueueTest, WhenPeekTheQueueReturnQueuedCommand) {
  Command command = {};
  Command *actual;
  underTest->enqueueCommand(&command);
  actual = underTest->peekCommand();
  ASSERT_THAT(actual, Eq(&command));
}

TEST_F(DeviceCommandQueueTest, WhenPeekAnEmptyQueueReturnNull) {
  Command *actual;
  actual = underTest->peekCommand();
  ASSERT_THAT(actual, IsNull());
}