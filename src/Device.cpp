#include <Device.h>

namespace Homio {

    Device::Device(Protocol *protocol, CommandPool *commandPool): protocol_(protocol), commandPool_(commandPool) {
        state_ = DeviceState::IDLE;
    }

    bool Device::enqueueCommand(Command *command) {
        if (commandQueueSize_ < HOMIO_COMMAND_QUEUE_SIZE) {
            uint8_t index = (commandQueueFirst_ + commandQueueSize_) % HOMIO_COMMAND_QUEUE_SIZE;
            commandQueue_[index] = command;
            commandQueueSize_ += 1;
            return true;
        }

        return false;
    }

    Command *Device::dequeueCommand() {
        Command *command;
        if (commandQueueSize_ > 0) {
            commandQueueSize_ -= 1;
            command = commandQueue_[commandQueueFirst_];
            commandQueueFirst_ = (commandQueueFirst_ + 1) % HOMIO_COMMAND_QUEUE_SIZE;
            return command;
        }

        return nullptr;
    }

    Command *Device::peekCommand() {
        if (commandQueueSize_ > 0) {
            return commandQueue_[commandQueueFirst_];
        }

        return nullptr;
    }

    void Device::tick() {
        switch (state_) {
            case DeviceState::IDLE:
                    if (commandQueueSize_ > 0) state_ = DeviceState::LOCK_REQUEST;
                break;
            case DeviceState::LOCK_REQUEST:
                    Command received = {};
                    Command command = {};
                    command.type = CommandType::LOCK_REQUEST;
                    if (protocol_->processCommand(&command, &received)) {

                    }
                    else {
                        state_ = DeviceState::IDLE;
                        if (received.fromAddress == 0 && received.fromAddress == 0) {
                            
                        }
                        else delay(100);
                    }
                break;
        }
    }

    bool Device::sendDatapoint(const uint8_t datapointId) {
        Datapoint *datapoint = getDatapoint(datapointId);

        if (datapoint) {
            Command *command = commandPool_->borrowCommandInstance();
            if (command == nullptr) return false;
            command->payloadSize = serializeDatapoint(datapointId, command->payload);
            return enqueueCommand(command);
        }

        return false;   
    }
}