#include <transport.h>
#include <utils.h>

namespace Homio {
    Transport::Transport(NRFLite *radio): radio_(radio) {
        
    }

    void Transport::addDatapoint(Datapoint *datapoint) {
        datapoints_[datapointsCount_] = datapoint;
        datapointsCount_ += 1;
    }

    Datapoint *Transport::getDatapoint(uint8_t datapointId) {
        for(uint8_t i = 0; i < getDatapointsCount(); i++) {
            if (datapoints_[i]->id == datapointId) {
                return datapoints_[i];
            }
        }

        return nullptr;
    }

    uint8_t Transport::getDatapointsCount() {
        return datapointsCount_;
    }

    bool Transport::sendCommand(const Command *command, const uint8_t radioId) {
        uint8_t dataSize = serializeCommand(command, buffer_);
        return radio_->send(radioId, buffer_, dataSize);
    }

    bool Transport::sendCommand(const Command *command) {
        return sendCommand(command, command->toAddress);
    }

    void Transport::receiveCommand(Command *command) {
        command->type = CommandType::CONFIRM;
    }

    bool Transport::receiveAck(Command *command) {
        if (radio_->hasAckData()) {
            radio_->readData(buffer_);

            unserializeCommand(command, buffer_);

            return true;
        }
        
        return false;
    }

    uint8_t Transport::serializeCommand(const Command *command, uint8_t *buffer) {
        buffer_[0] = static_cast<uint8_t>(command->type);
        buffer_[1] = command->fromAddress;
        buffer_[2] = command->toAddress;
        buffer_[3] = command->payloadSize;
        memcpy(buffer_ + HOMIO_COMMAND_HEADER_SIZE, command->payload, command->payloadSize);

        return HOMIO_COMMAND_HEADER_SIZE + command->payloadSize;
    }

    void Transport::unserializeCommand(Command *command, uint8_t *buffer) {
        command->type = (CommandType)buffer[0];
        command->fromAddress = buffer[1];
        command->toAddress = buffer[2];
        command->payloadSize = buffer[3];
    }

    uint8_t Transport::serializeDatapoint(uint8_t datapointId, uint8_t *buffer) {
        Datapoint *datapoint = getDatapoint(datapointId);
        uint8_t dataLength = 0;

        if (datapoint != nullptr) {
            buffer[0] = datapoint->id;
            buffer[1] = static_cast<uint8_t>(datapoint->type);
            switch(datapoint->type) {
                case DatapointType::BOOLEAN:
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE] = datapoint->value_bool;
                        dataLength = HOMIO_DATAPOINT_HEADER_SIZE + 1;
                    break;
                case DatapointType::INTEGER:
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE] = datapoint->value_uint >> 24;
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE + 1] = datapoint->value_uint >> 16;
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE + 2] = datapoint->value_uint >> 8;
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE + 3] = datapoint->value_uint >> 0;
                        dataLength = HOMIO_DATAPOINT_HEADER_SIZE + 4;
                    break;
                case DatapointType::BYTE:
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE] = datapoint->value_byte;
                        dataLength = HOMIO_DATAPOINT_HEADER_SIZE + 1;
                    break;
                case DatapointType::RAW:
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE] = datapoint->length;
                        memcpy(
                            buffer + HOMIO_DATAPOINT_HEADER_SIZE + 1, 
                            datapoint->value_raw,
                            datapoint->length < HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE - HOMIO_DATAPOINT_HEADER_SIZE 
                                ? datapoint->length : HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE - HOMIO_DATAPOINT_HEADER_SIZE
                        );
                        dataLength = HOMIO_DATAPOINT_HEADER_SIZE + datapoint->length;
                    break;
                case DatapointType::STRING:
                        buffer[HOMIO_DATAPOINT_HEADER_SIZE] = datapoint->length;
                        memcpy(
                            buffer + HOMIO_DATAPOINT_HEADER_SIZE + 1, 
                            datapoint->value_string,
                            datapoint->length < HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE - HOMIO_DATAPOINT_HEADER_SIZE 
                                ? datapoint->length : HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE - HOMIO_DATAPOINT_HEADER_SIZE
                        );
                        dataLength = HOMIO_DATAPOINT_HEADER_SIZE + datapoint->length;
                    break;
            }
        }

        return dataLength;
    }

    Datapoint *Transport::unserializeDatapoint(uint8_t *buffer) {
        Datapoint *datapoint = getDatapoint(buffer[0]);

        if (datapoint != nullptr) {
            datapoint->value_int = 0;
            switch(datapoint->type) {
                case DatapointType::BOOLEAN:
                        datapoint->value_bool = buffer[HOMIO_DATAPOINT_HEADER_SIZE];
                    break;
                case DatapointType::INTEGER:
                        datapoint->value_uint = encodeUint32(
                            buffer[HOMIO_DATAPOINT_HEADER_SIZE], 
                            buffer[HOMIO_DATAPOINT_HEADER_SIZE + 1], 
                            buffer[HOMIO_DATAPOINT_HEADER_SIZE + 2], 
                            buffer[HOMIO_DATAPOINT_HEADER_SIZE + 3]
                        );
                    break;
                case DatapointType::BYTE:
                        datapoint->value_byte = buffer[HOMIO_DATAPOINT_HEADER_SIZE];
                    break;
                case DatapointType::RAW:
                        memcpy(datapoint->value_raw, buffer + HOMIO_DATAPOINT_HEADER_SIZE + 1, buffer[HOMIO_DATAPOINT_HEADER_SIZE]);
                    break;
                case DatapointType::STRING:
                        memcpy(datapoint->value_string, buffer + HOMIO_DATAPOINT_HEADER_SIZE + 1, buffer[HOMIO_DATAPOINT_HEADER_SIZE]);
                    break;
            }
        }

        return datapoint;
    }
}