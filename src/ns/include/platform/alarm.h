#ifndef NS_PLATFORM_ALARM_H_
#define NS_PLATFORM_ALARM_H_

#include "ns/include/instance.h"

void
ns_plat_alarm_start_at(uint32_t t0, uint32_t dt);

void
ns_plat_alarm_stop(void);

uint32_t
ns_plat_alarm_get_now(void);

extern void
ns_plat_alarm_fired(ns_instance_t instance);

#endif // NS_PLATFORM_ALARM_H_
