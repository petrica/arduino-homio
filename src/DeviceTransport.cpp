#include <DeviceTransport.h>
#include <Utils.h>

namespace Homio {

    DeviceTransport::DeviceTransport(uint8_t deviceAddress, uint8_t hubAddress, NRFLite *radio, CommandPool *commandPool, CommandQueue *commandQueue): 
        deviceAddress_(deviceAddress), hubAddress_(hubAddress), radio_(radio), commandPool_(commandPool), commandQueue_(commandQueue)
        {

        state_ = TransportState::IDLE;
    }

    bool DeviceTransport::writeDatapoint(const Datapoint *datapoint) {
        Command *command = commandPool_->borrowCommandInstance();
        if (command == nullptr) return false;
        command->fromAddress = deviceAddress_;
        command->toAddress = hubAddress_;
        command->payloadSize = serializeDatapoint(datapoint, command->payload);
        return commandQueue_->enqueue(command);
    }

    bool DeviceTransport::sendCommand(const Command *command, const uint8_t radioId) {
        uint8_t buffer[HOMIO_BUFFER_SIZE];
        uint8_t dataSize = serializeCommand(command, buffer);
        return radio_->send(radioId, buffer, dataSize);
    }

    bool DeviceTransport::sendCommand(const Command *command) {
        return sendCommand(command, command->toAddress);
    }

    void DeviceTransport::receiveCommand(Command *command) {
        command->type = CommandType::CONFIRM;
    }

    bool DeviceTransport::receiveAck(Command *command) {
        if (radio_->hasAckData()) {
            uint8_t buffer[HOMIO_BUFFER_SIZE];
            radio_->readData(buffer);

            unserializeCommand(command, buffer);

            return true;
        }
        
        return false;
    }

    bool DeviceTransport::handleLockRequest(Command *receivedCommand) {
        Command command = {};
        command.type = CommandType::LOCK_REQUEST;
        command.fromAddress = deviceAddress_;
        command.toAddress = hubAddress_;
        
        return sendCommand(&command) 
            && receiveAck(receivedCommand) 
            && receivedCommand->payloadSize == 1;
    }

    bool DeviceTransport::handleCommand(Command *command) {
        if (command == nullptr) return false;

        Command received = {};
        return sendCommand(command, hubReceiveAddress_)
                && receiveAck(&received)
                && command->fromAddress == received.toAddress
                && received.type == CommandType::CONFIRM;
    }

    void DeviceTransport::tick() {
        Command *received;

        switch (state_) {
            case TransportState::IDLE:
                    if (commandQueue_->getSize() > 0) state_ = TransportState::LOCK_REQUEST;
                break;
            case TransportState::LOCK_REQUEST:
                    received = commandPool_->borrowCommandInstance();
                    if (received != nullptr && handleLockRequest(received)) {
                        if (received->toAddress == deviceAddress_) {
                                hubReceiveAddress_ = received->payload[0];
                                state_ = TransportState::DATA_SEND;
                        }
                        else state_ = TransportState::LOCK_DELAY;
                    }
                    else {
                        state_ = TransportState::IDLE;
                    }
                    commandPool_->returnCommandInstance(received);
                break;
            case TransportState::LOCK_DELAY:
                    delay(HOMIO_TRANSPORT_CONFLICT_DELAY);
                break;
            case TransportState::DATA_SEND:
                    if (handleCommand(commandQueue_->peek())) {
                        commandQueue_->dequeue();
                    }
                    state_ = TransportState::IDLE;
                break;
        }
    }
}