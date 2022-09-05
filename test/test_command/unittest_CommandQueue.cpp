#include <gmock/gmock.h>
#include <CommandQueue.h>
#include <Matchers.h>

using namespace ::testing;
using namespace Homio;

class CommandQueueTest : public Test {
  public:
    CommandQueue *underTest;

  void SetUp() {
    underTest = new CommandQueue();
  }

  void TearDown() {
    delete underTest;
    underTest = nullptr;
  }
};

TEST_F(CommandQueueTest, EnqueuingFirstCommandReturnsTrue) {
  Command command;
  bool actual = underTest->enqueue(&command);
  ASSERT_TRUE(actual);
}

TEST_F(CommandQueueTest, CommandQueueHasSizeOfOne) {
  Command command;
  underTest->enqueue(&command);
  uint8_t queueSize = underTest->getSize();
  ASSERT_THAT(queueSize, Eq(1));
}

TEST_F(CommandQueueTest, WhenStartedCommandQueueIsEmpty) {
  uint8_t queueSize = underTest->getSize();
  ASSERT_THAT(queueSize, Eq(0));
}

TEST_F(CommandQueueTest, WhenEnqueuingAndDequeuingShouldGiveTheSameCommand) {
  Command command = {};
  underTest->enqueue(&command);
  Command *actual;
  actual = underTest->dequeue();
  ASSERT_THAT(actual, Eq(&command));
}

TEST_F(CommandQueueTest, WhenDequeueEmptyQueueRemainsEmpty) {
  underTest->dequeue();
  uint8_t queueSize = underTest->getSize();
  ASSERT_THAT(queueSize, Eq(0));
}

TEST_F(CommandQueueTest, WhenEnqueueingMoreItemsFirstDequeuedIsTheFirstQueued) {
  Command commandFirst = {};
  Command commandSecond = {};
  underTest->enqueue(&commandFirst);
  underTest->enqueue(&commandSecond);
  Command *actual;
  actual = underTest->dequeue();
  ASSERT_THAT(actual, Eq(&commandFirst));
}

TEST_F(CommandQueueTest, WhenEnqueueingMoreItemsSecondDequeuedIsTheSecondQueued) {
  Command commandFirst = {};
  Command commandSecond = {};
  underTest->enqueue(&commandFirst);
  underTest->enqueue(&commandSecond);
  Command *actual;
  underTest->dequeue();
  actual = underTest->dequeue();
  ASSERT_THAT(actual, Eq(&commandSecond));
}

TEST_F(CommandQueueTest, WhenEnqueuingAndTheQueueIfFullReturnFalse) {
  Command command = {};
  for(uint8_t i = 0; i < HOMIO_COMMAND_QUEUE_SIZE; i++) {
    underTest->enqueue(&command);  
  }
  
  ASSERT_FALSE(underTest->enqueue(&command));
}

TEST_F(CommandQueueTest, WhenEnqueuingMoreThanTheQueueSizeQueueStillReturnsTheExepctedObject) {
  Command command = {};
  for(uint8_t i = 0; i < HOMIO_COMMAND_QUEUE_SIZE; i++) {
    underTest->enqueue(&command);  
  }
  underTest->dequeue();
  Command actual = {};
  underTest->enqueue(&actual);
  for(uint8_t i = 0; i < HOMIO_COMMAND_QUEUE_SIZE - 1; i++) {
    underTest->dequeue();  
  }

  ASSERT_THAT(underTest->dequeue(), Eq(&actual));
}

TEST_F(CommandQueueTest, WhenPeekTheQueueTheSizeRemainsTheSame) {
  Command command = {};
  underTest->enqueue(&command);
  underTest->peek();
  ASSERT_THAT(underTest->getSize(), Eq(1));
}

TEST_F(CommandQueueTest, WhenPeekTheQueueReturnQueuedCommand) {
  Command command = {};
  Command *actual;
  underTest->enqueue(&command);
  actual = underTest->peek();
  ASSERT_THAT(actual, Eq(&command));
}

TEST_F(CommandQueueTest, WhenPeekAnEmptyQueueReturnNull) {
  Command *actual;
  actual = underTest->peek();
  ASSERT_THAT(actual, IsNull());
}