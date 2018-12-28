#ifndef NS_CORE_COMMON_INSTANCE_H_
#define NS_CORE_COMMON_INSTANCE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ns/instance.h"
#include "core/core-config.h"
#include "core/common/code_utils.h"
#include "core/common/logging.h"
#include "core/common/notifier.h"
#include "core/common/timer.h"
#include "core/common/tasklet.h"
#include "core/common/message.h"
#include "core/utils/heap.h"
#if NS_RADIO || NS_ENABLE_RAW_LINK_API
#include "core/mac/link_raw.h"
#endif
#include "core/net/ip6.h"

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
    ip6_t ip6;
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

notifier_t *
instance_get_notifier(void *instance);

heap_t *
instance_get_heap(void *instance);

ip6_t *
instance_get_ip6(void *instance);

#endif // NS_CORE_COMMON_INSTANCE_H_
