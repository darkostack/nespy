#ifndef NS_PLATFORM_ALARM_MILLI_H_
#define NS_PLATFORM_ALARM_MILLI_H_

#include "ns/instance.h"

void
ns_plat_alarm_milli_start_at(ns_instance_t instance, uint32_t t0, uint32_t dt);

void
ns_plat_alarm_milli_stop(ns_instance_t instance);

uint32_t
ns_plat_alarm_milli_get_now(void);

extern void
ns_plat_alarm_milli_fired(ns_instance_t instance);

#endif // NS_PLATFORM_ALARM_MILLI_H_
