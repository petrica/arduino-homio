#include <Device.h>

namespace Homio {

    Device::Device(Transport *transport): transport_(transport) {
        state_ = DeviceState::IDLE;
    }

    bool Device::enqueueCommand(Command *command) {
        commandQueue_[commandQueueSize_] = command;
        commandQueueSize_ += 1;
        return true;
    }

    Command *Device::dequeueCommand() {
        if (commandQueueSize_ > 0) {
            commandQueueSize_ -= 1;
            return commandQueue_[commandQueueSize_];
        }

        return nullptr;
    }

    void Device::tick() {
        if (commandQueueSize_ > 0) {
            state_ = DeviceState::LOCK_REQUEST;
        }
    }

    bool Device::processCommand(const Command *command) {
        if (!transport_->sendCommand(command)) return false;
        Command receiveCommand;
        if (!transport_->receiveAck(&receiveCommand)) return false;
        if (receiveCommand.toAddress != command->fromAddress) return false;

        return true;
    }

    bool Device::sendDatapoint(const uint8_t datapointId) {
        Datapoint *datapoint = getDatapoint(datapointId);

        if (datapoint) {
            Command *command = borrowCommandInstance();
            command->payloadSize = serializeDatapoint(datapointId, command->payload);
            return enqueueCommand(command);
        }

        return false;   
    }
}