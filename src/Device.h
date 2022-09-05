#ifndef _HomioDevice_h_
#define _HomioDevice_h_

#include <Homio.h>
#include <Utils.h>
#ifndef UNIT_TEST
    #include <DeviceTransport.h>
#else
    #include <DeviceTransportMock.h>
#endif

namespace Homio {

    #ifdef UNIT_TEST
    class DeviceUnderTest;
    #endif

    class Device {

        public:
            Device(DeviceTransport *deviceTransport);

            void addDatapoint(Datapoint *datapoint);
            Datapoint *getDatapoint(uint8_t datapointId);
            bool sendDatapoint(const uint8_t datapointId);

            void tick();

        private:
            DeviceTransport *transport_;
            Datapoint *datapoints_[HOMIO_DEVICE_DATAPOINT_MAX_COUNT];
            uint8_t datapointsCount_ = 0;

        #ifdef UNIT_TEST
        friend class DeviceUnderTest;
        #endif
    };
}

#endif