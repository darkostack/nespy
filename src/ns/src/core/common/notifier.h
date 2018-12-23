#ifndef NS_CORE_COMMON_NOTIFIER_H_
#define NS_CORE_COMMON_NOTIFIER_H_

#include "core/core-config.h"
#include "ns/instance.h"
#include "core/common/tasklet.h"
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
    notifier_callback_handler_func_t handler;
    notifier_callback_t *next;
};

struct _notifier_external_callback {
    ns_state_changed_callback_func_t handler;
    void *context;
};

struct _notifier {
    ns_changed_flags_t flags_to_signal;
    ns_changed_flags_t signaled_flags;
    tasklet_t task;
    notifier_callback_t *callbacks;
    notifier_external_calllback_t external_callbacks[NOTIFIER_MAX_EXTERNAL_HANDLERS];
};

void
notifier_callback_ctor(notifier_callback_t *callback, notifier_callback_handler_func_t handler);

void
notifier_ctor(void *instance, notifier_t *notifier);

ns_error_t
notifier_register_callback(notifier_t *notifier, notifier_callback_t *callback);

void
notifier_remove_callback(notifier_t *notifier, notifier_callback_t *callback);

ns_error_t
notifier_register_external_callback(notifier_t *notifier,
                                    ns_state_changed_callback_func_t callback,
                                    void *context);

void
notifier_remove_external_callback(notifier_t *notifier,
                                  ns_state_changed_callback_func_t callback,
                                  void *context);

void
notifier_signal(notifier_t *notifier, ns_changed_flags_t flags);

void
notifier_signal_if_first(notifier_t *notifier, ns_changed_flags_t flags);

bool
notifier_is_pending(notifier_t *notifier);

bool
notifier_has_signaled(notifier_t *notifier, ns_changed_flags_t flags);

#endif // NS_CORE_COMMON_NOTIFIER_H_
