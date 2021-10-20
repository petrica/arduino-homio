#include <utils.h>

namespace Homio {

    uint32_t encodeUint32(uint8_t msb, uint8_t byte2, uint8_t byte3, uint8_t lsb) {
        return (uint32_t(msb) << 24) | (uint32_t(byte2) << 16) | (uint32_t(byte3) << 8) | uint32_t(lsb);
    }

}