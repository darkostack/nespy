#ifndef NS_CORE_COMMON_NOTIFIER_H_
#define NS_CORE_COMMON_NOTIFIER_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/instance.h"
#include "ns/sys/core/common/tasklet.h"
#include <stdbool.h>
#include <stdint.h>

enum {
    NOTIFIER_MAX_EXTERNAL_HANDLERS = NS_CONFIG_MAX_STATECHANGE_HANDLERS,
    NOTIFIER_FLAGS_STRING_BUFFER_SIZE = 128,
};

typedef struct _notifier_callback notifier_callback_t;
typedef struct _notifier_external_callback notifier_external_calllback_t;
typedef struct _notifier notifier_t;

typedef void (*notifier_callback_handler_func_t)(notifier_callback_t *callback, ns_changed_flags_t flags);

struct _notifier_callback {
    void *instance;
    notifier_callback_handler_func_t handler;
    notifier_callback_t *next;
};

struct _notifier_external_callback {
    ns_state_changed_callback_func_t handler;
    void *context;
};

struct _notifier_t {
    ns_changed_flags_t flags_to_signal;
    ns_changed_flags_t signaled_flags;
    tasklet_t task;
    notifier_callback_t *callbacks;
    notifier_external_calllback_t external_callbacks[NOTIFIER_MAX_EXTERNAL_HANDLERS];
};

#endif // NS_CORE_COMMON_NOTIFIER_H_
