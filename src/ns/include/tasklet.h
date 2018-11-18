#ifndef NS_TASKLET_H_
#define NS_TASKLET_H_

#include "ns/include/instance.h"

void ns_tasklet_process(ns_instance_t instance);
bool ns_tasklet_are_pending(ns_instance_t instance);
extern void ns_tasklet_signal_pending(ns_instance_t instance);

#endif // NS_TASKLET_H_
