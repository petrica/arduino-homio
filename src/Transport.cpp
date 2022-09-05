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
}