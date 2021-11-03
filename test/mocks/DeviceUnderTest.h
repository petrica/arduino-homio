#ifndef _HomioDeviceUnderTest_h_
#define _HomioDeviceUnderTest_h_

#include <Device.h>

namespace Homio {
    class DeviceUnderTest: public Device {
        public:
            DeviceUnderTest(Protocol *protocol, CommandPool *commandPool): Device(protocol, commandPool) {

            }

            bool processCommand(const Command *command, Command *receivedCommand) {
                return Device::processCommand(command, receivedCommand);
            }

            bool enqueueCommand(Command *command) {
                return Device::enqueueCommand(command);
            }

            Command *dequeueCommand() {
                return Device::dequeueCommand();
            }

            Command *peekCommand() {
                return Device::peekCommand();
            }

            uint8_t getCommandQueueSize() {
                return commandQueueSize_;
            }

            void setState(DeviceState state) {
                state_ = state;
            }

            DeviceState getState() {
                return state_;
            }
        };
}

#endif
