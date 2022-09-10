#include <CommandPool.h>

namespace Homio {
    CommandPool::CommandPool() {
        for(uint8_t i = 0; i < HOMIO_COMMAND_POOL_SIZE; i++) {
            pool_[i] = getInstance();
        }
        poolCount_ = HOMIO_COMMAND_POOL_SIZE;
    }

    Command *CommandPool::getInstance() {
        Command *command = new Command();
        command->payload = new uint8_t[HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE];
        return command;
    }

    Command *CommandPool::borrowCommandInstance() {
        if (poolCount_ > 0) {
            poolCount_ -= 1;
            return pool_[poolCount_];
        }

        return nullptr;
    }

    void CommandPool::returnCommandInstance(Command *commandInstance) {
        if (commandInstance != nullptr && poolCount_ < HOMIO_COMMAND_POOL_SIZE) { 
            poolCount_ += 1;
            pool_[poolCount_ - 1] = commandInstance;
        }
    }
}