#ifndef _HomioDeviceProtocol_h_
#define _HomioDeviceProtocol_h_

#include <Homio.h>
#include <Protocol.h>
#ifndef UNITTEST_PROTOCOL
    #include <Transport.h>
#else
    #include <TransportMock.h>
#endif

namespace Homio {
    class DeviceProtocol: public Protocol {
        public:
            DeviceProtocol(Transport *transport);
            bool processCommand(const Command *command, Command *receivedCommand);

        private:
            Transport *transport_;
    };
}

#endif