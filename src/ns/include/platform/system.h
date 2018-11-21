#ifndef NS_PLATFORM_SYSTEM_H_
#define NS_PLATFORM_SYSTEM_H_

#include "ns/include/instance.h"

void
ns_plat_sys_init(uint32_t node_id);

void
ns_plat_sys_process_drivers(ns_instance_t instance);

#endif // NS_PLATFORM_SYSTEM_H_
