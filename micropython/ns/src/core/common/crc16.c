#include "core/common/crc16.h"

void
crc16_ctor(crc16_t *crc16, crc16_polynomial_t polynomial)
{
    crc16->polynomial = (uint16_t)polynomial;
    crc16_init(crc16);
}

void
crc16_init(crc16_t *crc16)
{
    crc16->crc = 0;
}

void
crc16_update(crc16_t *crc16, uint8_t byte)
{
    uint8_t i;

    crc16->crc = crc16->crc ^ (uint16_t)(byte << 8);
    i = 8;

    do {
        if (crc16->crc & 0x8000) {
            crc16->crc = (uint16_t)(crc16->crc << 1) ^ crc16->polynomial;
        } else {
            crc16->crc = (uint16_t)(crc16->crc << 1);
        }
    } while (--i);
}

uint16_t
crc16_get(crc16_t *crc16)
{
    return crc16->crc;
}
