#include <Component.h>
#include <Utils.h>

namespace Homio {

    uint8_t Component::serializeDeviceCapabilities(DeviceCapabilities *capabilities, uint8_t *buffer) {
        buffer[0] = capabilities->heartbeatInterval >> 24;
        buffer[1] = capabilities->heartbeatInterval >> 16;
        buffer[2] = capabilities->heartbeatInterval >> 8;
        buffer[3] = capabilities->heartbeatInterval >> 0;
        buffer[4] = capabilities->canReceive;

        return 5;
    }

    void Component::unserializeDeviceCapabilities(uint8_t *buffer, DeviceCapabilities *capabilities) {
        capabilities->heartbeatInterval = encodeUint32(
            buffer[0], 
            buffer[1], 
            buffer[2], 
            buffer[3]
        );
        capabilities->canReceive = buffer[4];
    }

    DeviceCapabilities Component::getCapabilities() {
        return capabilities_;
    }

    void Component::setCapabilities(DeviceCapabilities capabilities) {
        memcpy(&capabilities_, &capabilities, sizeof(DeviceCapabilities));
    }

}