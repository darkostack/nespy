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
#include "ns/sys/core/common/timer.h"
#include "ns/sys/core/common/tasklet.h"
#include "ns/sys/core/common/message.h"

typedef struct _instance instance_t;

struct _instance {
    bool is_initialized;
    timer_scheduler_t timer_sched;
    tasklet_scheduler_t tasklet_sched;
    message_pool_t message_pool;
    // --- instance objects
    timer_scheduler_t (*get_timer_scheduler)(void);
    tasklet_scheduler_t (*get_tasklet_scheduler)(void);
    message_pool_t (*get_message_pool)(void);
};

instance_t *instance_init(void);
instance_t *instance_get(void);

#endif // NS_CORE_COMMON_INSTANCE_H_
