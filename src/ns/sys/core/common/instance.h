#ifndef NS_CORE_COMMON_INSTANCE_H_
#define NS_CORE_COMMON_INSTANCE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ns/include/error.h"
#include "ns/sys/core/core-config.h"
#include "ns/sys/core/common/code_utils.h"
#include "ns/sys/core/common/typedef.h"
#include "ns/sys/core/common/timer.h"
#include "ns/sys/core/common/tasklet.h"

struct _instance {
    bool is_initialized;
    timer_scheduler_t *timer_sched;
    tasklet_scheduler_t *tasklet_sched;
};

instance_t *instance_init(void);
instance_t *instance_get(void);
bool instance_is_initialized(instance_t *instance);

#endif // NS_CORE_COMMON_INSTANCE_H_
