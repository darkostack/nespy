#include "core/common/notifier.h"
#include "core/common/code_utils.h"
#include "core/common/logging.h"
#include "core/common/string.h"
#include <limits.h>

STRING(notifier_info, NOTIFIER_FLAGS_STRING_BUFFER_SIZE);

// --- private functions declarations
static void
handle_state_changed(void *tasklet);

static void
notifier_handle_state_changed(notifier_t *notifier);

static void
notifier_log_changed_flags(ns_changed_flags_t aflags);

static const char *
notifier_flag_to_string(ns_changed_flags_t aflags);

// --- notifier functions
void
notifier_callback_ctor(notifier_callback_t *callback, notifier_callback_handler_func_t handler)
{
    callback->handler = handler;
    callback->next = callback;
}

void
notifier_ctor(void *instance, notifier_t *notifier)
{
    notifier->flags_to_signal = 0;
    notifier->signaled_flags = 0;
    tasklet_ctor(instance, &notifier->task, &handle_state_changed, notifier);
    notifier->callbacks = NULL;
    for (unsigned int i = 0; i < NOTIFIER_MAX_EXTERNAL_HANDLERS; i++) {
        notifier->external_callbacks[i].handler = NULL;
        notifier->external_callbacks[i].context = NULL;
    }
}

ns_error_t
notifier_register_callback(notifier_t *notifier, notifier_callback_t *callback)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(callback->next == callback, error = NS_ERROR_ALREADY);
    callback->next = notifier->callbacks;
    notifier->callbacks = callback;
exit:
    return error;
}

void
notifier_remove_callback(notifier_t *notifier, notifier_callback_t *callback)
{
    VERIFY_OR_EXIT(notifier->callbacks != NULL);
    if (notifier->callbacks == callback) {
        notifier->callbacks = notifier->callbacks->next;
        EXIT_NOW();
    }
    for (notifier_callback_t *cb = notifier->callbacks; cb->next != NULL; cb = cb->next) {
        if (cb->next == callback) {
            cb->next = callback->next;
            EXIT_NOW();
        }
    }
exit:
    callback->next = callback;
}

ns_error_t
notifier_register_external_callback(notifier_t *notifier,
                                    ns_state_changed_callback_func_t callback,
                                    void *context)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(callback != NULL);

    notifier_external_calllback_t *unused_callback = NULL;

    for (unsigned int i = 0; i < NOTIFIER_MAX_EXTERNAL_HANDLERS; i++) {
        if (notifier->external_callbacks[i].handler == NULL) {
            VERIFY_OR_EXIT(notifier->external_callbacks[i].handler != callback, error = NS_ERROR_ALREADY);
            unused_callback = &notifier->external_callbacks[i];
            notifier->external_callbacks[i].handler = callback;
            notifier->external_callbacks[i].context = context;
        }
    }

    VERIFY_OR_EXIT(unused_callback != NULL, error = NS_ERROR_NO_BUFS);

exit:
    return error;
}

void
notifier_remove_external_callback(notifier_t *notifier,
                                  ns_state_changed_callback_func_t callback,
                                  void *context)
{
    VERIFY_OR_EXIT(callback != NULL);
    for (unsigned int i = 0; i < NOTIFIER_MAX_EXTERNAL_HANDLERS; i++) {
        notifier_external_calllback_t cb = notifier->external_callbacks[i];
        if ((cb.handler == callback) && (cb.context == context)) {
            cb.handler = NULL;
            cb.context = NULL;
        }
    }
exit:
    return;
}

void
notifier_signal(notifier_t *notifier, ns_changed_flags_t flags)
{
    notifier->flags_to_signal |= flags;
    notifier->signaled_flags |= flags;
    tasklet_post(&notifier->task);
}

void
notifier_signal_if_first(notifier_t *notifier, ns_changed_flags_t flags)
{
    if (!notifier_has_signaled(notifier, flags)) {
        notifier_signal(notifier, flags);
    }
}

bool
notifier_is_pending(notifier_t *notifier)
{
    return (notifier->flags_to_signal != 0);
}

bool
notifier_has_signaled(notifier_t *notifier, ns_changed_flags_t flags)
{
    return (notifier->signaled_flags & flags) == flags;
}

// --- private functions
static void
handle_state_changed(void *tasklet)
{
    notifier_handle_state_changed((notifier_t *)((tasklet_t *)tasklet)->handler.context);
}

static void
notifier_handle_state_changed(notifier_t *notifier)
{
    ns_changed_flags_t flags = notifier->flags_to_signal;

    VERIFY_OR_EXIT(flags != 0);

    notifier->flags_to_signal = 0;

    notifier_log_changed_flags(flags);

    for (notifier_callback_t *callback = notifier->callbacks; callback != NULL; callback = callback->next) {
        if (callback->handler != NULL) {
            callback->handler(callback, flags);
        }
    }

    for (unsigned int i = 0; i < NOTIFIER_MAX_EXTERNAL_HANDLERS; i++) {
        notifier_external_calllback_t callback = notifier->external_callbacks[i];
        if (callback.handler != NULL) {
            callback.handler(flags, callback.context);
        }
    }

exit:
    return;
}

#if (NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO)
static void
notifier_log_changed_flags(ns_changed_flags_t aflags)
{
    ns_changed_flags_t flags = aflags;
    bool is_first = true;
    string_t *notifier_string = &notifier_info_string;
    for (uint8_t bit = 0; bit < sizeof(ns_changed_flags_t) * CHAR_BIT; bit++) {
        VERIFY_OR_EXIT(flags != 0);
        if (flags & (1 << bit)) {
            SUCCESS_OR_EXIT(string_append(notifier_string, "%s%s",
                                          is_first ? "" : " ",
                                          notifier_flag_to_string(1 << bit)));
            is_first = false;
            flags ^= (1 << bit);
        }
    }

exit:
    ns_log_info_core("notifier: state-changed (0x%04x) [%s] ", aflags, string_as_c_string(notifier_string));
}

static const char *
notifier_flag_to_string(ns_changed_flags_t aflags)
{
    const char *retval = "(unknown)";
    switch (aflags) {
    case NS_CHANGED_IP6_ADDRESS_ADDED:
        retval = "ip6+";
        break;
    case NS_CHANGED_IP6_ADDRESS_REMOVED:
        retval = "ip6-";
        break;
    case NS_CHANGED_THREAD_ROLE:
        retval = "role";
        break;
    case NS_CHANGED_THREAD_LL_ADDR:
        retval = "lladdr";
        break;
    case NS_CHANGED_THREAD_ML_ADDR:
        retval = "mladdr";
        break;
    case NS_CHANGED_THREAD_RLOC_ADDED:
        retval = "rloc+";
        break;
    case NS_CHANGED_THREAD_RLOC_REMOVED:
        retval = "rloc-";
        break;
    case NS_CHANGED_THREAD_PARTITION_ID:
        retval = "partitionid";
        break;
    case NS_CHANGED_THREAD_KEY_SEQUENCE_COUNTER:
        retval = "keyseqcntr";
        break;
    case NS_CHANGED_THREAD_NETDATA:
        retval = "netdata";
        break;
    case NS_CHANGED_THREAD_CHILD_ADDED:
        retval = "child+";
        break;
    case NS_CHANGED_THREAD_CHILD_REMOVED:
        retval = "child-";
        break;
    case NS_CHANGED_IP6_MULTICAST_SUBSRCRIBED:
        retval = "ip6mult+";
        break;
    case NS_CHANGED_IP6_MULTICAST_UNSUBSRCRIBED:
        retval = "ip6mult-";
        break;
    case NS_CHANGED_COMMISSIONER_STATE:
        retval = "commissionerstate";
        break;
    case NS_CHANGED_JOINER_STATE:
        retval = "joinerstate";
        break;
    case NS_CHANGED_THREAD_CHANNEL:
        retval = "channel";
        break;
    case NS_CHANGED_THREAD_PANID:
        retval = "panid";
        break;
    case NS_CHANGED_THREAD_NETWORK_NAME:
        retval = "netname";
        break;
    case NS_CHANGED_THREAD_EXT_PANID:
        retval = "extpanid";
        break;
    case NS_CHANGED_MASTER_KEY:
        retval = "mstrkey";
        break;
    case NS_CHANGED_PSKC:
        retval = "pskc";
        break;
    case NS_CHANGED_SECURITY_POLICY:
        retval = "secpolicy";
        break;
    case NS_CHANGED_CHANNEL_MANAGER_NEW_CHANNEL:
        retval = "cmnewchan";
        break;
    case NS_CHANGED_SUPPORTED_CHANNEL_MASK:
        retval = "chanmask";
        break;
    default:
        break;
    }
    return retval;
}
#else //#if (NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO)
static void
notifier_log_changed_flags(ns_changed_flags_t)
{
}

static const char *
notifier_flag_to_string(ns_changed_flags_t)
{
    return "";
}
#endif //#if (NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO)
