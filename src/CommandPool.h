#ifndef _HomioComponentPool_h_
#define _HomioComponentPool_h_

#include <Homio.h>

namespace Homio {
    #ifdef UNIT_TEST
    class CommandPoolUnderTest;
    #endif

    class CommandPool {
        public:
            CommandPool();

            Command *borrowCommandInstance();
            void returnCommandInstance(Command *commandInstance);

        private:
            Command *getInstance();

            Command *pool_[HOMIO_COMMAND_POOL_SIZE];
            uint8_t poolCount_;

        #ifdef UNIT_TEST
        friend class CommandPoolUnderTest;
        #endif
    };
}

#endif
