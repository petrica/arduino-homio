#include <Component.h>
#include <Utils.h>

namespace Homio {

    void Component::addDatapoint(Datapoint *datapoint) {
        datapoints_[datapointsCount_] = datapoint;
        datapointsCount_ += 1;
    }

    Datapoint *Component::getDatapoint(uint8_t datapointId) {
        for(uint8_t i = 0; i < datapointsCount_; i++) {
            if (datapoints_[i]->id == datapointId) {
                return datapoints_[i];
            }
        }

        return nullptr;
    }

    uint8_t Component::serializeDatapoint(uint8_t datapointId, uint8_t *buffer) {
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

    Datapoint *Component::unserializeDatapoint(uint8_t *buffer) {
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

    uint8_t Component::serializeDeviceCapabilities(DeviceCapabilities *capabilities, uint8_t *buffer) {
        buffer[0] = capabilities->heartbeatInterval >> 24;
        buffer[1] = capabilities->heartbeatInterval >> 16;
        buffer[2] = capabilities->heartbeatInterval >> 8;
        buffer[3] = capabilities->heartbeatInterval >> 0;
        buffer[4] = capabilities->canReceive;

        return 5;
    }

    void Component::unserializeDeviceCapabilities(uint8_t *buffer, DeviceCapabilities *capabilities) {
        capabilities->heartbeatInterval = encodeUint32(
            buffer[0], 
            buffer[1], 
            buffer[2], 
            buffer[3]
        );
        capabilities->canReceive = buffer[4];
    }

    DeviceCapabilities Component::getCapabilities() {
        return capabilities_;
    }

    void Component::setCapabilities(DeviceCapabilities capabilities) {
        memcpy(&capabilities_, &capabilities, sizeof(DeviceCapabilities));
    }

}