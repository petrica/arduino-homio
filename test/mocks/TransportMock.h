#ifndef _HomioTransport_h_
#define _HomioTransport_h_

#include <ArduinoFake.h>
#include <gmock/gmock.h>
#include <Homio.h>

namespace Homio {
    class Transport {
        public:
            Transport() {};
            virtual ~Transport() = default;
            virtual bool sendCommand(const Command *command) = 0;
            // virtual bool sendCommand(const Command *command, const uint8_t radioId);
            // virtual void receiveCommand(Command *command);
            virtual bool receiveAck(Command *command) = 0;
    };

    class TransportMock: public Transport {
        public:
            TransportMock(): Transport() {};
            MOCK_METHOD(bool, sendCommand, (const Command *command), (override));
            MOCK_METHOD(bool, receiveAck, (Command *command), (override));
    };
}

#endif