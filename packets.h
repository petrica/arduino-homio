#ifndef HOMIO_PACKETS_H_
#define HOMIO_PACKETS_H_

#define HOMIO_NRF24_BUFFER_SIZE 32

#include <Arduino.h>
#include <vector>
#include <string>

namespace homio {

    enum class CommandType : uint8_t {
        HEARTBEAT = 0x00,
        LOCK_REQUEST = 0x01,
        LOCK_DELIVER = 0x02,
        DATAPOINT_REPORT = 0x03,
        DATAPOINT_DELIVER = 0x04,
        CONFIRM = 0x05
    };

    struct Command {
        CommandType type;
        uint16_t fromAddress;
        uint16_t toAddress;
        std::vector<uint8_t> payload;
    };

    enum class DatapointType : uint8_t {
        RAW = 0x00,      // variable length
        BOOLEAN = 0x01,  // 1 byte (0/1)
        INTEGER = 0x02,  // 4 byte
        STRING = 0x03,   // variable length
        ENUM = 0x04,     // 1 byte
        BITMASK = 0x05,  // 2 bytes
    };

    struct Datapoint {
        uint8_t id;
        DatapointType type;
        size_t len;
        union {
            bool value_bool;
            int value_int;
            uint32_t value_uint;
            uint8_t value_enum;
            uint32_t value_bitmask;
        };
        std::string value_string;
        std::vector<uint8_t> value_raw;
    };


    
}

#endif