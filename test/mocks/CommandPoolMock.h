#ifndef _HomioCommandPool_h_
#define _HomioCommandPool_h_

#include <Arduino.h>
#include <gmock/gmock.h>
#include <Homio.h>

namespace Homio {
    class CommandPool {
        public:
            CommandPool() {};
            virtual ~CommandPool() = default;
            virtual Command *borrowCommandInstance() = 0;
            virtual void returnCommandInstance(Command *commandInstance) = 0;
    };

    class CommandPoolMock: public CommandPool {
        public:
            CommandPoolMock(): CommandPool() {};
            MOCK_METHOD(Command*, borrowCommandInstance, (), (override));
            MOCK_METHOD(void, returnCommandInstance, (Command *commandInstance), (override));
    };
}

#endif