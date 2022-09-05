#ifndef _HomioDeviceTransport_h_
#define _HomioDeviceTransport_h_

#include <Arduino.h>
#include <gmock/gmock.h>
#include <Homio.h>

namespace Homio {
    class DeviceTransport {
        public:
            DeviceTransport() {};
            virtual ~DeviceTransport() = default;

            virtual bool writeDatapoint(const Datapoint *datapoint) = 0;
            virtual uint8_t getDatapointId() = 0;
            virtual bool readDatapoint(Datapoint *datapoint) = 0;

            virtual void tick() = 0;
    };

    class DeviceTransportMock: public DeviceTransport {
        public:
            DeviceTransportMock(): DeviceTransport() {};
            MOCK_METHOD(bool, writeDatapoint, (const Datapoint *datapoint), (override));
            MOCK_METHOD(uint8_t, getDatapointId, (), (override));
            MOCK_METHOD(bool, readDatapoint, (Datapoint *datapoint), (override));
            MOCK_METHOD(void, tick, (), (override));
    };
}

#endif