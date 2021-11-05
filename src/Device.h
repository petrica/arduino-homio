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
            bool sendLockRequest(Command *receiveCommand);
            bool sendCommand(Command *command);

            Command *commandQueue_[HOMIO_COMMAND_QUEUE_SIZE];
            uint8_t commandQueueSize_ = 0;
            uint8_t commandQueueFirst_ = 0;

            DeviceState state_;

            Transport *transport_;
            CommandPool *commandPool_;

            const uint8_t deviceAddress_;
            const uint8_t hubAddress_;
            uint8_t hubReceiveAddress_;

        #ifdef UNIT_TEST
        friend class DeviceUnderTest;
        #endif
    };
}

#endif