#include <transport.h>
#include <utils.h>

namespace Homio {
    Transport::Transport(NRFLite *radio): radio_(radio) {
        
    }

    bool Transport::sendCommand(const Command *command, const uint8_t radioId) {
        uint8_t dataSize = serializeCommand(command, buffer_);
        return radio_->send(radioId, buffer_, dataSize);
    }

    bool Transport::sendCommand(const Command *command) {
        return sendCommand(command, command->toAddress);
    }

    void Transport::receiveCommand(Command *command) {
        command->type = CommandType::CONFIRM;
    }

    bool Transport::receiveAck(Command *command) {
        if (radio_->hasAckData()) {
            radio_->readData(buffer_);

            unserializeCommand(command, buffer_);

            return true;
        }
        
        return false;
    }

    uint8_t Transport::serializeCommand(const Command *command, uint8_t *buffer) {
        buffer_[0] = static_cast<uint8_t>(command->type);
        buffer_[1] = command->fromAddress;
        buffer_[2] = command->toAddress;
        buffer_[3] = command->payloadSize;
        memcpy(buffer + HOMIO_COMMAND_HEADER_SIZE, command->payload, command->payloadSize);

        return HOMIO_COMMAND_HEADER_SIZE + command->payloadSize;
    }

    void Transport::unserializeCommand(Command *command, uint8_t *buffer) {
        command->type = (CommandType)buffer[0];
        command->fromAddress = buffer[1];
        command->toAddress = buffer[2];
        command->payloadSize = buffer[3];
    }
}