#ifndef _HomioDevice_h_
#define _HomioDevice_h_

#include <Homio.h>
#include <Utils.h>
#include <Component.h>
#ifndef UNITTEST_DEVICE
    #include <Transport.h>
    #include <CommandPool.h>
#else
    #include <TransportMock.h>
    #include <CommandPoolMock.h>
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
            Device(uint8_t deviceAddress, uint8_t hubAddress, Transport *transport, CommandPool *commandPool);

            bool sendDatapoint(const uint8_t datapointId);

            void tick();
            
        protected:
            bool enqueueCommand(Command *command);
            Command *dequeueCommand();
            Command *peekCommand();

        private:
            Command *commandQueue_[HOMIO_COMMAND_QUEUE_SIZE];
            uint8_t commandQueueSize_ = 0;
            uint8_t commandQueueFirst_ = 0;

            DeviceState state_;

            Transport *transport_;
            CommandPool *commandPool_;

            const uint8_t deviceAddress_;
            const uint8_t hubAddress_;

        #ifdef UNIT_TEST
        friend class DeviceUnderTest;
        #endif
    };
}

#endif