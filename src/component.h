#ifndef _HomioComponent_h_
#define _HomioComponent_h_

#include <Homio.h>

namespace Homio {
    #ifdef UNIT_TEST
    class ComponentUnderTest;
    #endif

    class Component {

        public:
            void addDatapoint(Datapoint *datapoint);
            Datapoint *getDatapoint(uint8_t datapointId);

        protected:
            uint8_t serializeDatapoint(uint8_t datapointId, uint8_t *buffer);
            Datapoint *unserializeDatapoint(uint8_t *buffer);

        private:
            Datapoint *datapoints_[5];
            uint8_t datapointsCount_ = 0;

        #ifdef UNIT_TEST
        friend class ComponentUnderTest;
        #endif
    };
}

#endif