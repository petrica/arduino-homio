#ifndef HOMIO_DEVICE_H_
#define HOMIO_DEVICE_H_

#include <Arduino.h>
#include <NRFLite.h>
#include <vector>
#include <packets.h>
#include <component.h>
#include <config.h>

namespace homio {

    class Device: public Component {
        public:
            Device(uint8_t deviceId, uint8_t hubId, uint8_t cePin, uint8_t csnPin);
            void begin();
            void tick();
            void setDatapointValue(uint8_t id, uint32_t value);

            enum class DeviceState : uint8_t {
                IDLE = 0x00,
                LOCK_REQUEST = 0x01,
                LOCK_DELAY = 0x02,
                DATA_SEND = 0x03
            };

        protected:
            bool sendRawCommand(const Command& command, std::vector<uint8_t>& buffer);

        private:
            uint8_t deviceId_;
            uint8_t hubId_;
            NRFLite *radio_;
            uint8_t cePin_;
            uint8_t csnPin_;
            uint8_t channel_ = 100;

            bool hasFatalError_ = false;

            DeviceState state_;
    };
}

#endif