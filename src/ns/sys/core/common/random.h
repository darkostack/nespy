#ifndef NS_CORE_COMMON_RANDOM_H_
#define NS_CORE_COMMON_RANDOM_H_

#include <stdint.h>
#include "ns/sys/core/core-config.h"

uint8_t
random_get_uint8(void);

uint16_t
random_get_uint16(void);

uint32_t
random_get_uint32(void);

uint8_t
random_get_uint8_in_range(uint8_t min, uint8_t max);

uint16_t
random_get_uint16_in_range(uint16_t min, uint16_t max);

uint32_t
random_get_uint32_in_range(uint32_t min, uint32_t max);

void
random_fill_buffer(uint8_t *buffer, uint16_t size);

uint32_t
random_add_jitter(uint32_t value, uint16_t jitter);

#endif // NS_CORE_COMMON_RANDOM_H_
