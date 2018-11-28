#ifndef NS_CORE_COMMON_INSTANCE_H_
#define NS_CORE_COMMON_INSTANCE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ns/sys/core/core-config.h"
#include "ns/sys/core/common/code_utils.h"
#include "ns/sys/core/common/timer.h"
#include "ns/sys/core/common/tasklet.h"
#include "ns/sys/core/common/message.h"
#include "ns/sys/core/utils/heap.h"

typedef struct _instance instance_t;

struct _instance {
    bool is_initialized;
    timer_scheduler_t timer_sched;
    tasklet_scheduler_t tasklet_sched;
    message_pool_t message_pool;
    heap_t heap;
};

instance_t *
instance_init(void);

instance_t *
instance_get(void);

timer_scheduler_t *
instance_get_timer_scheduler(instance_t *instance);

tasklet_scheduler_t *
instance_get_tasklet_scheduler(instance_t *instance);

message_pool_t *
instance_get_message_pool(void);

heap_t *
instance_get_heap(instance_t *instance);

#endif // NS_CORE_COMMON_INSTANCE_H_
