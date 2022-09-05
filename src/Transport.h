#ifndef _HomioTransport_h_
#define _HomioTransport_h_

#include <Homio.h>
#include <NRFLite.h>

namespace Homio {

    class Transport
    {
        public:
            Transport(NRFLite *radio);

            bool sendCommand(const Command *command);
            bool sendCommand(const Command *command, const uint8_t radioId);
            void receiveCommand(Command *command);
            bool receiveAck(Command *command);

        private:
            NRFLite *radio_;
    };

}

#endif