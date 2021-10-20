#include <device.h>

namespace Homio {

    Device::Device(Transport *transport): transport_(transport) {
        setState(DeviceState::IDLE);
    }

    void Device::enqueueCommand(Command *command) {
        commandQueue_[commandQueueSize_] = command;
        commandQueueSize_ += 1;
    }

    Command *Device::dequeueCommand() {
        if (commandQueueSize_ > 0) {
            commandQueueSize_ -= 1;
            return commandQueue_[commandQueueSize_];
        }

        return nullptr;
    }

    uint8_t Device::getCommandQueueSize() {
        return commandQueueSize_;
    }

    void Device::setState(DeviceState state) {
        state_ = state;
    }

    DeviceState Device::getState() {
        return state_;
    }

    void Device::tick() {
        if (getCommandQueueSize() > 0) {
            setState(DeviceState::LOCK_REQUEST);
        }
    }

    bool Device::processCommand(const Command *command) {
        if (!transport_->sendCommand(command)) return false;
        Command receiveCommand;
        if (!transport_->receiveAck(&receiveCommand)) return false;
        if (receiveCommand.toAddress != command->fromAddress) return false;

        return true;
    }
}