#include <Utils.h>

namespace Homio {

    uint32_t encodeUint32(uint8_t msb, uint8_t byte2, uint8_t byte3, uint8_t lsb) {
        return (uint32_t(msb) << 24) | (uint32_t(byte2) << 16) | (uint32_t(byte3) << 8) | uint32_t(lsb);
    }

    uint8_t serializeCommand(const Command *command, uint8_t *buffer) {
        buffer[0] = static_cast<uint8_t>(command->type);
        buffer[1] = command->fromAddress;
        buffer[2] = command->toAddress;
        buffer[3] = command->payloadSize;
        memcpy(buffer + HOMIO_COMMAND_HEADER_SIZE, command->payload, command->payloadSize);

        return HOMIO_COMMAND_HEADER_SIZE + command->payloadSize;
    }

    void unserializeCommand(Command *command, uint8_t *buffer) {
        command->type = (CommandType)buffer[0];
        command->fromAddress = buffer[1];
        command->toAddress = buffer[2];
        command->payloadSize = buffer[3];
        if (command->payloadSize && command->payload != nullptr) {
            memcpy(command->payload, buffer + HOMIO_COMMAND_HEADER_SIZE, min(command->payloadSize, HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE));
        }
    }

    uint8_t serializeDatapoint(const Datapoint *datapoint, uint8_t *buffer) {
        uint8_t dataLength = 0;

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

        return dataLength;
    }

    void unserializeDatapoint(Datapoint *datapoint, uint8_t *buffer) {
        datapoint->id = buffer[0];

        datapoint->value_int = 0;
        datapoint->type = static_cast<DatapointType>(buffer[1]);
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
}