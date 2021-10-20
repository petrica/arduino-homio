#ifndef _HomioDevice_h_
#define _HomioDevice_h_

#include <homio.h>
#include <utils.h>
#ifndef UNITTEST_DEVICE
    #include <transport.h>
#else
    #include <TransportMock.h>
#endif

namespace Homio {

    enum class DeviceState : uint8_t {
        IDLE = 0x00,
        LOCK_REQUEST = 0x01,
        LOCK_DELAY = 0x02,
        DATA_SEND = 0x03
    };

    class Device {
        public:
            Device(Transport *transport);

            void enqueueCommand(Command *command);

            Command *dequeueCommand();

            uint8_t getCommandQueueSize();

            void setState(DeviceState state);

            DeviceState getState();

            void tick();

            bool processCommand(const Command *command);

        private:
            Command *commandQueue_[5];
            uint8_t commandQueueSize_ = 0;

            DeviceState state_;

            Transport *transport_;
    };
}

#endif