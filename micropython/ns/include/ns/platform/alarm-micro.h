#ifndef NS_PLATFORM_ALARM_MICRO_H_
#define NS_PLATFORM_ALARM_MICRO_H_

#include <stdint.h>
#include "ns/instance.h"

void
ns_plat_alarm_micro_start_at(ns_instance_t instance, uint32_t t0, uint32_t dt);

void
ns_plat_alarm_micro_stop(ns_instance_t instance);

uint32_t
ns_plat_alarm_micro_get_now(void);

extern void
ns_plat_alarm_micro_fired(ns_instance_t instance);

#endif // NS_PLATFORM_ALARM_MICRO_H_
