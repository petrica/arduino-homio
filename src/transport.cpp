#include <Transport.h>
#include <Utils.h>

namespace Homio {
    Transport::Transport(NRFLite *radio): radio_(radio) {
        
    }

    bool Transport::sendCommand(const Command *command, const uint8_t radioId) {
        uint8_t buffer[HOMIO_BUFFER_SIZE];
        uint8_t dataSize = serializeCommand(command, buffer);
        return radio_->send(radioId, buffer, dataSize);
    }

    bool Transport::sendCommand(const Command *command) {
        return sendCommand(command, command->toAddress);
    }

    void Transport::receiveCommand(Command *command) {
        command->type = CommandType::CONFIRM;
    }

    bool Transport::receiveAck(Command *command) {
        if (radio_->hasAckData()) {
            uint8_t buffer[HOMIO_BUFFER_SIZE];
            radio_->readData(buffer);

            unserializeCommand(command, buffer);

            return true;
        }
        
        return false;
    }

    uint8_t Transport::serializeCommand(const Command *command, uint8_t *buffer) {
        buffer[0] = static_cast<uint8_t>(command->type);
        buffer[1] = command->fromAddress;
        buffer[2] = command->toAddress;
        buffer[3] = command->payloadSize;
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