#ifndef NS_CORE_COMMON_INSTANCE_H_
#define NS_CORE_COMMON_INSTANCE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ns/core/core-config.h"
#include "ns/core/common/typedef.h"
#include "ns/core/common/timer.h"

struct _instance {
    bool is_initialized;
    timer_scheduler_t *timer_sched;
};

instance_t *instance_init(void);
instance_t *instance_get(void);
bool instance_is_initialized(instance_t *instance);

#endif // NS_CORE_COMMON_INSTANCE_H_
