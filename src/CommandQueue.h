#ifndef _HomioCommandQueue_h_
#define _HomioCommandQueue_h_

#include <Homio.h>

namespace Homio {

    class CommandQueue {
        public:
            CommandQueue();

            bool enqueue(Command *command);
            uint8_t getSize();
            Command *dequeue();
            Command *peek();

        private:
            Command *queue_[HOMIO_COMMAND_QUEUE_SIZE];
            uint8_t queueSize_ = 0;
            uint8_t queueFirst_ = 0;

    };
}

#endif