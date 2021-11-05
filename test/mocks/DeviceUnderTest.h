#ifndef _HomioDeviceUnderTest_h_
#define _HomioDeviceUnderTest_h_

#include <Device.h>

namespace Homio {
    class DeviceUnderTest: public Device {
        public:
            DeviceUnderTest(Transport *transport, CommandPool *commandPool): 
                Device(10, 1, transport, commandPool) {

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

            uint8_t getHubReceiveAddress() {
                return hubReceiveAddress_;
            }

            void setHubReceiveAddress(uint8_t address) {
                hubReceiveAddress_ = address;
            }
        };
}

#endif
