#ifndef NS_CORE_COMMON_INSTANCE_H_
#define NS_CORE_COMMON_INSTANCE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ns/include/instance.h"
#include "ns/sys/core/core-config.h"
#include "ns/sys/core/common/code_utils.h"
#include "ns/sys/core/common/logging.h"
#include "ns/sys/core/common/notifier.h"
#include "ns/sys/core/common/timer.h"
#include "ns/sys/core/common/tasklet.h"
#include "ns/sys/core/common/message.h"
#include "ns/sys/core/utils/heap.h"
#if NS_RADIO || NS_ENABLE_RAW_LINK_API
#include "ns/sys/core/mac/link_raw.h"
#endif

typedef struct _instance instance_t;

struct _instance {
    bool is_initialized;
    timer_scheduler_t timer_milli_scheduler;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_scheduler_t timer_micro_scheduler;
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    tasklet_scheduler_t tasklet_scheduler;
    ns_log_level_t log_level;
    notifier_t notifier;
    message_pool_t message_pool;
#if NS_RADIO || NS_ENABLE_RAW_LINK_API
    mac_link_raw_t link_raw;
#endif
    heap_t heap;
};

instance_t *
instance_ctor(void);

instance_t *
instance_get(void);

ns_log_level_t
instance_get_log_level(void);

#if NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL
void
instance_set_log_level(ns_log_level_t log_level);
#endif

#endif // NS_CORE_COMMON_INSTANCE_H_
