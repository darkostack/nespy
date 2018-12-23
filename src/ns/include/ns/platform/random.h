#ifndef NS_PLATFORM_RANDOM_H_
#define NS_PLATFORM_RANDOM_H_

#include <stdint.h>
#include "ns/error.h"

uint32_t
ns_plat_random_get(void);

ns_error_t
ns_plat_random_get_true(uint8_t *output, uint16_t output_length);

#endif // NS_PLATFORM_RANDOM_H_
