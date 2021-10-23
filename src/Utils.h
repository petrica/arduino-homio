#ifndef _HomioUtils_h_
#define _HomioUtils_h_

#include <Arduino.h>

namespace Homio {

    uint32_t encodeUint32(uint8_t msb, uint8_t byte2, uint8_t byte3, uint8_t lsb);
    
}

#endif