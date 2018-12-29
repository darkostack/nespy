#ifndef NS_CORE_COMMON_CRC16_H_
#define NS_CORE_COMMON_CRC16_H_

#include "core/core-config.h"
#include <stdint.h>

typedef enum _crc16_polynomial {
    CRC16_POLYNOMIAL_CCITT = 0x1021,
    CRC16_POLYNOMIAL_ANSI  = 0x8005,
} crc16_polynomial_t;

typedef struct _crc16 {
    uint16_t polynomial;
    uint16_t crc;
} crc16_t;

void
crc16_ctor(crc16_t *crc16, crc16_polynomial_t polynomial);

void
crc16_init(crc16_t *crc16);

void
crc16_update(crc16_t *crc16, uint8_t byte);

uint16_t
crc16_get(crc16_t *crc16);

#endif // NS_CORE_COMMON_CRC16_H_
