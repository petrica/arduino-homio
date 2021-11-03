#ifndef _HomioProtocol_h_
#define _HomioProtocol_h_

#include <Homio.h>

namespace Homio {
    class Protocol {
        public:
            virtual bool processCommand(const Command *command, Command *receivedCommand) = 0;
    };
}

#endif