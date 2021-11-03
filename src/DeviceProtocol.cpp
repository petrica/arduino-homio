#include <DeviceProtocol.h>

namespace Homio {
    DeviceProtocol::DeviceProtocol(Transport *transport): transport_(transport) {

    };

    bool DeviceProtocol::processCommand(const Command *command, Command *receivedCommand) {
        if (!transport_->sendCommand(command)) return false;
        if (!transport_->receiveAck(receivedCommand)) return false;
        if (receivedCommand->toAddress != command->fromAddress) return false;

        return true;
    };
}