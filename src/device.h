#ifndef _HomioDevice_h_
#define _HomioDevice_h_

#include <homio.h>
#include <utils.h>
#include <component.h>
#ifndef UNITTEST_DEVICE
    #include <transport.h>
#else
    #include <TransportMock.h>
#endif

namespace Homio {

    #ifdef UNIT_TEST
    class DeviceUnderTest;
    #endif

    enum class DeviceState : uint8_t {
        IDLE = 0x00,
        LOCK_REQUEST = 0x01,
        LOCK_DELAY = 0x02,
        DATA_SEND = 0x03
    };

    class Device: public Component {

        public:
            Device(Transport *transport);

            void tick();
            
        protected:
            bool processCommand(const Command *command);

            void enqueueCommand(Command *command);

            Command *dequeueCommand();

        private:
            Command *commandQueue_[5];
            uint8_t commandQueueSize_ = 0;

            DeviceState state_;

            Transport *transport_;

        #ifdef UNIT_TEST
        friend class DeviceUnderTest;
        #endif
    };
}

#endif