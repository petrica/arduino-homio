#ifndef _HomioDeviceTransportUnderTest_h_
#define _HomioDeviceTransportUnderTest_h_

#include <DeviceTransport.h>

namespace Homio {
    class DeviceTransportUnderTest: public DeviceTransport {
        public:
            DeviceTransportUnderTest(uint8_t deviceAddress, uint8_t hubAddress, NRFLite *radio, CommandPool *commandPool, CommandQueue *commandQueue):
                DeviceTransport(deviceAddress, hubAddress, radio, commandPool, commandQueue) {

            }

            bool sendCommand(const Command *command) {
                return DeviceTransport::sendCommand(command);
            }

            bool sendCommand(const Command *command, const uint8_t radioId) {
                return DeviceTransport::sendCommand(command, radioId);
            }

            void receiveCommand(Command *command) {
                DeviceTransport::receiveCommand(command);
            }

            bool receiveAck(Command *command) {
                return DeviceTransport::receiveAck(command);
            }

            void setState(TransportState state) {
                state_ = state;
            }

            TransportState getState() {
                return state_;
            }

            uint8_t getHubReceiveAddress() {
                return hubReceiveAddress_;
            }

            void setHubReceiveAddress(uint8_t hubReceiveAddress) {
                hubReceiveAddress_ = hubReceiveAddress;
            }

            uint8_t getDatapointId() {
                return DeviceTransport::getDatapointId();
            }

            bool handleCommand(Command *command) {
                return DeviceTransport::handleCommand(command);
            }

            void setReceivedCommand(Command *command) {
                receivedCommand_ = command;
            }

            Command *getReceivedCommand() {
                return receivedCommand_;
            }
    };
}

#endif