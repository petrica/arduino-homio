#include <CommandQueue.h>

namespace Homio {

    CommandQueue::CommandQueue() {
        
    }

    bool CommandQueue::enqueue(Command *command) {
        if (queueSize_ < HOMIO_COMMAND_QUEUE_SIZE) {
            uint8_t index = (queueFirst_ + queueSize_) % HOMIO_COMMAND_QUEUE_SIZE;
            queue_[index] = command;
            queueSize_ += 1;
            return true;
        }

        return false;
    }

    Command *CommandQueue::dequeue() {
        Command *command;
        if (queueSize_ > 0) {
            queueSize_ -= 1;
            command = queue_[queueFirst_];
            queueFirst_ = (queueFirst_ + 1) % HOMIO_COMMAND_QUEUE_SIZE;
            return command;
        }

        return nullptr;
    }

    Command *CommandQueue::peek() {
        if (queueSize_ > 0) {
            return queue_[queueFirst_];
        }

        return nullptr;
    }

    uint8_t CommandQueue::getSize() {
        return queueSize_;
    }
}