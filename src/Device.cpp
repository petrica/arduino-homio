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
        Command received = {};
        Command command = {};

        switch (state_) {
            case DeviceState::IDLE:
                    if (commandQueueSize_ > 0) state_ = DeviceState::LOCK_REQUEST;
                break;
            case DeviceState::LOCK_REQUEST:
                    command.type = CommandType::LOCK_REQUEST;
                    if (protocol_->processCommand(&command, &received)) {
                        state_ = DeviceState::DATA_SEND;
                    }
                    else if (received.fromAddress == 0 && received.fromAddress == 0) state_ = DeviceState::IDLE;    
                    else state_ = DeviceState::LOCK_DELAY;
                break;
            case DeviceState::LOCK_DELAY:
                    delay(100);
                break;
            case DeviceState::DATA_SEND:
                    Command *commandToSend = peekCommand();
                    if (commandToSend != nullptr) {
                        if (protocol_->processCommand(commandToSend, &received))
                            dequeueCommand();
                    }
                    state_ = DeviceState::IDLE;
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