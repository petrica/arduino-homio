#ifndef HOMIO_UTILS_H_
#define HOMIO_UTILS_H_

#include <Arduino.h>
#include <vector>
#include <packets.h>

namespace homio {
    void debugBuffer(std::vector<uint8_t> &buffer);

    void debugDatapoint(Datapoint& datapoint);

    uint32_t encodeUint32(uint8_t msb, uint8_t byte2, uint8_t byte3, uint8_t lsb);
}

#endif
