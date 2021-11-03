#ifndef _HomioProtocol_h_
#define _HomioProtocol_h_

#include <Arduino.h>
#include <gmock/gmock.h>
#include <Homio.h>

namespace Homio {
    class Protocol {
        public:
            Protocol() {};
            virtual ~Protocol() = default;
            virtual bool processCommand(const Command *command, Command *receivedCommand) = 0;
    };

    class ProtocolMock: public Protocol {
        public:
            ProtocolMock(): Protocol() {};
            MOCK_METHOD(bool, processCommand, (const Command *command, Command *receivedCommand), (override));
    };
}

#endif