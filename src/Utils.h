#ifndef _HomioUtils_h_
#define _HomioUtils_h_

#include <Arduino.h>
#include <Homio.h>

namespace Homio {

    uint32_t encodeUint32(uint8_t msb, uint8_t byte2, uint8_t byte3, uint8_t lsb);
    
    uint8_t serializeCommand(const Command *command, uint8_t *buffer);
    void unserializeCommand(Command *command, uint8_t *buffer);

    uint8_t serializeDatapoint(const Datapoint *datapoint, uint8_t *buffer);
    void unserializeDatapoint(Datapoint *datapoint, uint8_t *buffer);
}

#endif