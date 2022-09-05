#ifndef _HomioDeviceTransport_h_
#define _HomioDeviceTransport_h_

#include <Homio.h>
#include <NRFLite.h>
#include <Utils.h>
#ifndef UNIT_TEST
    #include <CommandPool.h>
    #include <CommandQueue.h>
#else
    #include <CommandPoolMock.h>
    #include <CommandQueueMock.h>    
#endif

namespace Homio {

    #ifdef UNIT_TEST
    class DeviceTransportUnderTest;
    #endif

    class DeviceTransport
    {
        public:
            DeviceTransport(uint8_t deviceAddress, uint8_t hubAddress, NRFLite *radio, CommandPool *commandPool, CommandQueue *commandQueue);

            bool writeDatapoint(const Datapoint *datapoint);
            uint8_t getDatapointId();
            bool readDatapoint(Datapoint *datapoint);

            void tick();

        protected:
            bool sendCommand(const Command *command);
            bool sendCommand(const Command *command, const uint8_t radioId);
            void receiveCommand(Command *command);
            bool receiveAck(Command *command);

            bool handleLockRequest(Command *receiveCommand);
            bool handleCommand(Command *command);

        private:
            NRFLite *radio_;
            CommandPool *commandPool_;
            CommandQueue *commandQueue_;

            const uint8_t deviceAddress_;
            const uint8_t hubAddress_;
            uint8_t hubReceiveAddress_;

            TransportState state_;

        #ifdef UNIT_TEST
        friend class DeviceTransportUnderTest;
        #endif
    };

}

#endif