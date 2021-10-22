#ifndef _HomioDeviceUnderTest_h_
#define _HomioDeviceUnderTest_h_

#include <device.h>

namespace Homio {
    class DeviceUnderTest: public Device {
        public:
            DeviceUnderTest(Transport *transport): Device(transport) {

            }

            bool processCommand(const Command *command) {
            return Device::processCommand(command);
            }

            void enqueueCommand(Command *command) {
            Device::enqueueCommand(command);
            }

            Command *dequeueCommand() {
            return Device::dequeueCommand();
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
