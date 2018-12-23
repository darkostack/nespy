#ifndef NS_PLATFORM_TIME_H_
#define NS_PLATFORM_TIME_H_

#include <stdint.h>

uint64_t
ns_plat_time_get(void);

uint16_t
ns_plat_time_get_xtal_accuracy(void);

#endif // NS_PLATFORM_TIME_H_
