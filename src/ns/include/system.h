#ifndef NS_SYSTEM_H_
#define NS_SYSTEM_H_

#include "ns/include/instance.h"

void ns_sys_init(uint32_t node_id);
void ns_sys_process_drivers(ns_instance_t instance);

#endif // NS_SYSTEM_H_
