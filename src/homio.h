#ifndef _HomioHomio_h_
#define _HomioHomio_h_

#include <config.h>
#include <Arduino.h>

namespace Homio
{
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
        uint8_t fromAddress;
        uint8_t toAddress;
        uint8_t *payload;
        uint8_t payloadSize = 0;
    };

    enum class DatapointType : uint8_t {
        RAW = 0x00,      // variable length
        BOOLEAN = 0x01,  // 1 byte (0/1)
        INTEGER = 0x02,  // 4 byte
        BYTE = 0x04,     // 1 byte
        STRING = 0x03,   // variable length        
    };

    struct Datapoint {
        uint8_t id;
        DatapointType type;
        size_t length;
        union {
            bool value_bool;
            int value_int;
            uint32_t value_uint;
            uint8_t value_byte;
        };
        char *value_string;
        uint8_t *value_raw;
    };
}

#endif