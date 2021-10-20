#ifndef _HomioTransport_h_
#define _HomioTransport_h_

#include <homio.h>
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

            void addDatapoint(Datapoint *datapoint);
            Datapoint *getDatapoint(uint8_t datapointId);
            uint8_t getDatapointsCount();

            uint8_t serializeDatapoint(uint8_t datapointId, uint8_t *buffer);
            Datapoint *unserializeDatapoint(uint8_t *buffer);

        protected:
            uint8_t serializeCommand(const Command *command, uint8_t *buffer);
            void unserializeCommand(Command *command, uint8_t *buffer);

        private:
            NRFLite *radio_;

            Datapoint *datapoints_[5];
            uint8_t datapointsCount_ = 0;

            uint8_t buffer_[HOMIO_BUFFER_SIZE];
    };

}

#endif