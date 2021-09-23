#include <component.h>
#include <utils.h>

namespace homio 
{
    void Component::addDatapoint(Datapoint& datapoint) {
        datapoints_.push_back(datapoint);
    }

    Datapoint& Component::getDatapoint(uint8_t id) {
        for (auto &datapoint : datapoints_) {
            if (datapoint.id == id)
            return datapoint;
        }

        return datapoints_[0];
    }

    bool Component::packDatapointValue(uint8_t id, uint32_t value, std::vector<uint8_t>& buffer) {
        Datapoint datapoint = getDatapoint(id);
        if (datapoint.id != id) {
            Serial.println(F("Invalid datapoint"));
            return false;
        }
        DatapointType allowedTypes[] = { 
            DatapointType::ENUM, 
            DatapointType::INTEGER, 
            DatapointType::BOOLEAN,
            DatapointType::BITMASK
        };
        bool found = false;
        for(uint8_t i = 0; i < sizeof(allowedTypes); i++) {
            if (allowedTypes[i] == datapoint.type) found = true;
        }
        if (!found) {
            Serial.println(F("Datapoint type not supported, expected ENUM, INTEGER, BOOLEAN or BITMASK"));
            return false;
        }

        datapoint.value_uint = value;
        std::vector<uint8_t> data;
        switch (datapoint.len) {
            case 4:
                data.push_back(value >> 24);
                data.push_back(value >> 16);
            case 2:
                data.push_back(value >> 8);
            case 1:
                data.push_back(value >> 0);
            break;
            default:
                Serial.println(F("Unexpected datapoint length"));
                return false;
        }

        packDatapoint(datapoint.id, datapoint.type, data, buffer);

        return true;
    }

    bool Component::packDatapointValue(uint8_t id, std::string& value, std::vector<uint8_t>& buffer) {
        Datapoint datapoint = getDatapoint(id);
        if (datapoint.id != id) {
            Serial.println(F("Invalid datapoint"));
            return false;
        }
        if (datapoint.type != DatapointType::STRING) {
            Serial.println(F("Datapoint type not supported, expected STRING type."));
            return false;
        }     

        std::vector<uint8_t> data;
        for (char const &c : value) {
            data.push_back(c);
        }
        packDatapoint(datapoint.id, datapoint.type, data, buffer);

        return true;
    }

    bool Component::packDatapointValue(uint8_t id, std::vector<uint8_t>& value, std::vector<uint8_t>& buffer) {
        Datapoint datapoint = getDatapoint(id);
        if (datapoint.id != id) {
            Serial.println(F("Invalid datapoint"));
            return false;
        }
        if (datapoint.type != DatapointType::RAW) {
            Serial.println(F("Datapoint type not supported, expected RAW type."));
            return false;
        }     

        packDatapoint(datapoint.id, datapoint.type, value, buffer);

        return true;
    }

    void Component::packDatapoint(uint8_t id, DatapointType datapointType, std::vector<uint8_t>& data, std::vector<uint8_t>& buffer) {
        buffer.push_back(id);
        buffer.push_back(static_cast<uint8_t>(datapointType));
        buffer.push_back((uint8_t)data.size());
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    bool Component::unpackDatapoint(std::vector<uint8_t>& buffer, Datapoint& datapoint) {
        if (buffer.size() < 3) {
            Serial.println(F("Buffer too short for any data to unpack"));
            return false;
        }

        datapoint.id = buffer[0];
        datapoint.type = (DatapointType) buffer[1];
        datapoint.value_uint = 0;
        datapoint.len = buffer[2];

        switch (datapoint.type) {
            case DatapointType::RAW:
                    datapoint.value_raw = std::vector<uint8_t>(buffer.begin() + 3, buffer.begin() + 3 + datapoint.len);
                break;
            case DatapointType::BOOLEAN:
                    datapoint.value_bool = buffer[3];
                break;
            case DatapointType::INTEGER:
                    datapoint.value_uint = encodeUint32(buffer[3], buffer[4], buffer[5], buffer[6]);
                break;
            case DatapointType::STRING:
                    datapoint.value_string = std::string(reinterpret_cast<const char *>(buffer.begin() + 3), datapoint.len);
                break;
            case DatapointType::ENUM:
                    datapoint.value_enum = buffer[3];
                break;
            case DatapointType::BITMASK:
                switch (datapoint.len) {
                    case 1:
                            datapoint.value_bitmask = encodeUint32(0, 0, 0, buffer[3]);
                        break;
                    case 2:
                            datapoint.value_bitmask = encodeUint32(0, 0, buffer[3], buffer[4]);
                        break;
                    case 4:
                            datapoint.value_bitmask = encodeUint32(buffer[3], buffer[4], buffer[5], buffer[6]);
                        break;
                    default:
                            Serial.println(F("Datapoint has bad bitmask length."));
                        return false;
                }
                break;
            default:
                    Serial.println(F("Datapoint has unknown datatype."));
                return false;
        }

        return true;
    }

    void Component::queueCommand(const Command command) {
        Serial.println(F("=== Queue command ==="));
        Serial.print(F("Type: "));
        Serial.println((uint8_t)command.type);
        Serial.print(F("From address: "));
        Serial.println(command.fromAddress);
        Serial.print(F("To address: "));
        Serial.println(command.toAddress);
        Serial.print(F("Payload size: "));
        Serial.println(command.payload.size());

        commandQueue_.push_back(command);

        Serial.println(F("=== End Queue command ==="));
    }

    void Component::processCommandQueue() {
        sendCommand(commandQueue_.front());
        commandQueue_.erase(commandQueue_.begin());
    }

    size_t Component::getCommandQueueSize() {
        return commandQueue_.size();
    }

    bool Component::sendCommand(const Command& command) {
        lastCommandTimestamp_ = millis();

        Serial.println(F("=== Send command ==="));
        Serial.print(F("Type: "));
        Serial.println((uint8_t)command.type);
        Serial.print(F("From address: "));
        Serial.println(command.fromAddress);
        Serial.print(F("To address: "));
        Serial.println(command.toAddress);
        Serial.print(F("Payload size: "));
        Serial.println(command.payload.size());

        std::vector<uint8_t> raw;
        raw.push_back(static_cast<uint8_t>(command.type));
        raw.push_back(command.fromAddress);
        raw.push_back(command.toAddress);
        raw.push_back(command.payload.size());
        raw.insert(raw.end(), command.payload.begin(), command.payload.end());

        Serial.print("Payload: ");
        debugBuffer(raw);
        Serial.println("=== End Send command ===");

        return sendRawCommand(command, raw);
    }

    void Component::handleRawCommand(std::vector<uint8_t> buffer, Command& command) {
        Serial.println(F("=== Handle Raw Command ==="));

        command.type = (CommandType)buffer[0];
        command.fromAddress = buffer[1];
        command.toAddress = buffer[2];
        uint8_t payloadSize = buffer[3];
        command.payload.insert(command.payload.begin(), buffer.begin() + 4, buffer.begin() + 4 + payloadSize);

        Serial.print(F("Type: "));
        Serial.println((uint8_t)command.type);
        Serial.print(F("From address: "));
        Serial.println(command.fromAddress);
        Serial.print(F("To address: "));
        Serial.println(command.toAddress);
        Serial.print(F("Payload size: "));
        Serial.println(command.payload.size());
        Serial.print("Payload: ");
        debugBuffer(buffer);

        Serial.println(F("=== End Handle Raw Command ==="));
    }
}