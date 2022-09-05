#ifndef _HomioComponent_h_
#define _HomioComponent_h_

#include <Homio.h>

namespace Homio {
    #ifdef UNIT_TEST
    class ComponentUnderTest;
    #endif

    class Component {

        public:
            void setCapabilities(DeviceCapabilities capabilities);
            DeviceCapabilities getCapabilities();

        protected:
            uint8_t serializeDeviceCapabilities(DeviceCapabilities *capabilities, uint8_t *buffer);
            void unserializeDeviceCapabilities(uint8_t *buffer, DeviceCapabilities *capabilities);

        private:
            Datapoint *datapoints_[5];
            uint8_t datapointsCount_ = 0;
            DeviceCapabilities capabilities_;

        #ifdef UNIT_TEST
        friend class ComponentUnderTest;
        #endif
    };
}

#endif