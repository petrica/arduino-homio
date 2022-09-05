#ifndef _HomioCommandQueue_h_
#define _HomioCommandQueue_h_

#include <gmock/gmock.h>
#include <Homio.h>

namespace Homio {
    class CommandQueue {
        public:
            CommandQueue() {};
            virtual ~CommandQueue() = default;

            virtual bool enqueue(Command *command) = 0;
            virtual uint8_t getSize() = 0;
            virtual Command *dequeue() = 0;
            virtual Command *peek() = 0;
    };

    class CommandQueueMock: public CommandQueue {
        public:
            CommandQueueMock(): CommandQueue() {};
            MOCK_METHOD(bool, enqueue, (Command *command), (override));
            MOCK_METHOD(uint8_t, getSize, (), (override));
            MOCK_METHOD(Command*, dequeue, (), (override));
            MOCK_METHOD(Command*, peek, (), (override));
    };
}

#endif