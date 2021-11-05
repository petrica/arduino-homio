#ifndef _HomioComponentUnderTest_h_
#define _HomioComponentUnderTest_h_

#include <Component.h>

namespace Homio {
    class ComponentUnderTest: public Component {
    public:
        uint8_t serializeDatapoint(uint8_t datapointId, uint8_t *buffer) {
            return Component::serializeDatapoint(datapointId, buffer);
        }

        Datapoint *unserializeDatapoint(uint8_t *buffer) {
            return Component::unserializeDatapoint(buffer);
        }

        uint8_t getDatapointsCount() {
            return datapointsCount_;
        }

        uint8_t serializeDeviceCapabilities(DeviceCapabilities *capabilities, uint8_t *buffer) {
            return Component::serializeDeviceCapabilities(capabilities, buffer);
        }
        
        void unserializeDeviceCapabilities(uint8_t *buffer, DeviceCapabilities *capabilities) {
            Component::unserializeDeviceCapabilities(buffer, capabilities);
        }
    };
}

#endif
