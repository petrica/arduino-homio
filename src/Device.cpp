#include <Device.h>

namespace Homio {

    Device::Device(uint8_t deviceAddress, uint8_t hubAddress, Transport *transport, CommandPool *commandPool): 
        transport_(transport), commandPool_(commandPool), deviceAddress_(deviceAddress), hubAddress_(hubAddress) {
        state_ = DeviceState::IDLE;
        setCapabilities({
            HOMIO_DEVICE_CAPABILITIES_HEARTBEAT_INTERVAL,
            HOMIO_DEVICE_CAPABILITIES_CAN_RECEIVE
        });
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
        Command *received;

        switch (state_) {
            case DeviceState::IDLE:
                    if (commandQueueSize_ > 0) state_ = DeviceState::LOCK_REQUEST;
                break;
            case DeviceState::LOCK_REQUEST:
                    received = commandPool_->borrowCommandInstance();
                    if (sendLockRequest(received)) {
                        if (received->toAddress == deviceAddress_) {
                                hubReceiveAddress_ = received->payload[0];
                                state_ = DeviceState::DATA_SEND;
                        }
                        else state_ = DeviceState::LOCK_DELAY;
                    }
                    else {
                        state_ = DeviceState::IDLE;
                    }
                    commandPool_->returnCommandInstance(received);
                break;
            case DeviceState::LOCK_DELAY:
                    delay(100);
                break;
            case DeviceState::DATA_SEND:
                    if (sendCommand(peekCommand())) {
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
            command->fromAddress = deviceAddress_;
            command->toAddress = hubAddress_;
            command->payloadSize = serializeDatapoint(datapointId, command->payload);
            return enqueueCommand(command);
        }

        return false;   
    }

    bool Device::sendLockRequest(Command *receivedCommand) {
        Command command = {};
        command.type = CommandType::LOCK_REQUEST;
        command.fromAddress = deviceAddress_;
        command.toAddress = hubAddress_;
        
        return transport_->sendCommand(&command) 
            && transport_->receiveAck(receivedCommand) 
            && receivedCommand->payloadSize == 1;
    }

    bool Device::sendCommand(Command *command) {
        if (command == nullptr) return false;

        Command received = {};
        return transport_->sendCommand(command, hubReceiveAddress_)
                && transport_->receiveAck(&received)
                && command->fromAddress == received.toAddress
                && received.type == CommandType::CONFIRM;
    }
}