#ifndef _HomioDevice_h_
#define _HomioDevice_h_

#include <Homio.h>
#include <Utils.h>
#include <Component.h>
#ifndef UNITTEST_DEVICE
    #include <Protocol.h>
    #include <CommandPool.h>
#else
    #include <ProtocolMock.h>
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
            Device(Protocol *protocol, CommandPool *commandPool);

            bool sendDatapoint(const uint8_t datapointId);

            void tick();
            
        protected:
            bool processCommand(const Command *command, Command *receivedCommand);

            bool enqueueCommand(Command *command);
            Command *dequeueCommand();
            Command *peekCommand();

        private:
            Command *commandQueue_[HOMIO_COMMAND_QUEUE_SIZE];
            uint8_t commandQueueSize_ = 0;
            uint8_t commandQueueFirst_ = 0;

            DeviceState state_;

            Protocol *protocol_;
            CommandPool *commandPool_;

        #ifdef UNIT_TEST
        friend class DeviceUnderTest;
        #endif
    };
}

#endif