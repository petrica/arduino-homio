#ifndef HOMIO_COMPONENT_H_
#define HOMIO_COMPONENT_H_

#include <Arduino.h>
#include <packets.h>

namespace homio
{
    class Component {
        public:
            void addDatapoint(Datapoint& datapoint);
            Datapoint& getDatapoint(uint8_t id);
            size_t getCommandQueueSize();

        protected:
            bool packDatapointValue(uint8_t id, uint32_t value, std::vector<uint8_t>& buffer);
            bool packDatapointValue(uint8_t id, std::vector<uint8_t>& value, std::vector<uint8_t>& buffer);
            bool packDatapointValue(uint8_t id, std::string& valuem, std::vector<uint8_t>& buffer);
            bool unpackDatapoint(std::vector<uint8_t>& buffer, Datapoint& datapoint);
            virtual bool sendRawCommand(const Command &command, std::vector<uint8_t> &buffer) = 0;
            void processCommandQueue();
            bool sendCommand(const Command &command);
            void handleRawCommand(std::vector<uint8_t> buffer, Command& command);
            void queueCommand(const Command command);
        private:
            void packDatapoint(uint8_t id, DatapointType datapointType, std::vector<uint8_t>& data, std::vector<uint8_t>& buffer);
            std::vector<Datapoint> datapoints_;
            std::vector<Command> commandQueue_;
            uint32_t lastCommandTimestamp_ = 0;
    };
}

#endif