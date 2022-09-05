#ifndef _HomioDeviceUnderTest_h_
#define _HomioDeviceUnderTest_h_

#include <Device.h>

namespace Homio {
    class DeviceUnderTest: public Device {
        public:
            DeviceUnderTest(DeviceTransport *deviceTransport): 
                Device(deviceTransport) {

            }

            uint8_t getDatapointsCount() {
                return datapointsCount_;
            }
        };
}

#endif
