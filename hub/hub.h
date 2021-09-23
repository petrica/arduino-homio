#ifndef HOMIO_HUB_H_
#define HOMIO_HUB_H_

#include <Arduino.h>
#include <RF24.h>
#include <vector>
#include <hub/hub_device.h>
#include <packets.h>
#include <component.h>
#include <config.h>

namespace homio {

    class Hub: public Component {

        struct Lock {
            bool isLocked;
            uint8_t deviceId;          
        };

        public:
            Hub(uint8_t deviceId, uint8_t cePin, uint8_t csnPin);
            void begin();
            void tick();

        protected:
            bool sendRawCommand(const Command& command, std::vector<uint8_t>& buffer);

        private:
            uint8_t deviceId_;
            uint8_t address_[5] = { 1, 2, 3, 4, 5 };
            uint8_t addressData_[5] = { 1, 2, 3, 4, 5 };
            uint8_t otherAddress_[5] = { 2, 3, 4, 5, 6};
            uint8_t channel_ = 100;
            uint8_t cePin_;
            uint8_t csnPin_;

            RF24 *radio_;
            bool hasFatalError_ = false;

            std::vector<HubDevice*> devices;

            Command command_{};

            Lock lock{false, 0};

    };

}

#endif