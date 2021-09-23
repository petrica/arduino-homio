#include <utils.h>

namespace homio {
    void debugBuffer(std::vector<uint8_t> &buffer) {
        for(uint16_t i = 0; i < buffer.size(); i ++)
        {
            Serial.print((uint8_t)buffer[i]);
            if (i < buffer.size() - 1) Serial.print(";");
        }
        Serial.println("");
    }

    void debugDatapoint(Datapoint& datapoint) {
        Serial.println("=== Datapoint ===");
        Serial.print("ID: ");
        Serial.println(datapoint.id);
        Serial.print("Type: ");
        switch (datapoint.type)
        {
            case DatapointType::RAW:
                    Serial.println(F("RAW"));
                break;
            case DatapointType::BOOLEAN:
                    Serial.println(F("BOOLEAN"));
                break;
            case DatapointType::INTEGER:
                    Serial.println(F("INTEGER"));
                break;
            case DatapointType::STRING:
                    Serial.println(F("STRING"));
                break;
            case DatapointType::ENUM:
                    Serial.println(F("ENUM"));
                break;
            case DatapointType::BITMASK:
                    Serial.println(F("BITMASK"));
                break;        
            default:
                    Serial.println(F("UNKNOWN"));
                break;
        }
        Serial.print("Value: ");
        switch (datapoint.type)
        {
            case DatapointType::RAW:
                    debugBuffer(datapoint.value_raw);
                break;
            case DatapointType::BOOLEAN:
                    if (datapoint.value_bool) Serial.println(F("true"));
                    else Serial.println(F("false"));
                break;
            case DatapointType::INTEGER:
                    Serial.println(datapoint.value_int);
                    Serial.print("Value unsigned: ");
                    Serial.println(datapoint.value_uint);
                break;
            case DatapointType::STRING:
                    Serial.println(datapoint.value_string.c_str());
                break;
            case DatapointType::ENUM:
                    Serial.println(datapoint.value_enum);
                break;
            case DatapointType::BITMASK:
                    Serial.println(datapoint.value_bitmask, BIN);
                break;
            default:
                    Serial.println(F("UNKNOWN"));
                break;

        }
        Serial.println("=== End Datapoint ===");
    }

    uint32_t encodeUint32(uint8_t msb, uint8_t byte2, uint8_t byte3, uint8_t lsb) {
        return (uint32_t(msb) << 24) | (uint32_t(byte2) << 16) | (uint32_t(byte3) << 8) | uint32_t(lsb);
    }
}