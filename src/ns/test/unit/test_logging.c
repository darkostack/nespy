#include "ns/sys/core/core-config.h"
#include "ns/include/platform/random.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/utils/heap.h"

#define RUN_TEST 0

static void
print_logging(void)
{
#if RUN_TEST
    ns_log_crit_api("print test 0x%08x", 0xdeadbeef);

    uint8_t dump_buffer[128];

    for (unsigned i = 0; i < sizeof(dump_buffer); i++)
    {
        dump_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    ns_dump_crit_mac("test buffer", dump_buffer, sizeof(dump_buffer));
#endif
}

#if RUN_TEST
void
channel_changed_callback(notifier_callback_t *callback, ns_changed_flags_t flags)
{
    VERIFY_OR_EXIT(flags & NS_CHANGED_THREAD_CHANNEL);

    printf("notifier: CHANGED THREAD CHANNEL\r\n");

    heap_t *heap = &((instance_t *)instance_get())->heap;

    heap_free(heap, callback);

    if (heap_is_clean(heap)) {
        printf("heap is clean now\r\n");
    }

exit:
    return;
}

void
child_added_callback(notifier_callback_t *callback, ns_changed_flags_t flags)
{
    VERIFY_OR_EXIT(flags & NS_CHANGED_THREAD_CHILD_ADDED);

    printf("notifier: CHANGED THREAD CHILD ADDED\r\n");

    heap_t *heap = &((instance_t *)instance_get())->heap;

    heap_free(heap, callback);

    if (heap_is_clean(heap)) {
        printf("heap is clean now\r\n");
    }

exit:
    return;
}

static void
external_netdata_changed_callback(ns_changed_flags_t flags, void *context)
{
    VERIFY_OR_EXIT(flags & NS_CHANGED_THREAD_NETDATA);

    printf("external notifier: CHANGED THREAD NETDATA\r\n");

exit:
    return;
}
#endif

static void
test_notifier(void)
{
#if RUN_TEST
    instance_t *instance = instance_get();
    heap_t *heap = &instance->heap;

    // signal if first should only print one time
    notifier_signal_if_first(instance_get_notifier(instance), NS_CHANGED_THREAD_PANID);
    notifier_signal_if_first(instance_get_notifier(instance), NS_CHANGED_THREAD_PANID);
    notifier_signal_if_first(instance_get_notifier(instance), NS_CHANGED_THREAD_PANID);

    // register notifier callback
    notifier_callback_t *channel_changed;
    notifier_callback_t *child_added;

    channel_changed = (notifier_callback_t *)heap_calloc(heap, 1, sizeof(notifier_callback_t));
    child_added = (notifier_callback_t *)heap_calloc(heap, 1, sizeof(notifier_callback_t));

    notifier_callback_ctor(channel_changed, &channel_changed_callback);
    notifier_callback_ctor(child_added, &child_added_callback);

    notifier_register_callback(instance_get_notifier(instance), channel_changed);
    notifier_register_callback(instance_get_notifier(instance), child_added);

    notifier_signal(instance_get_notifier(instance), NS_CHANGED_THREAD_CHANNEL);
    notifier_signal(instance_get_notifier(instance), NS_CHANGED_THREAD_CHILD_ADDED);

    // register external notifier callback
    ns_error_t error = NS_ERROR_NONE;
    error = notifier_register_external_callback(instance_get_notifier(instance),
                                                &external_netdata_changed_callback,
                                                NULL);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);


    // test the failed case
    error = notifier_register_external_callback(instance_get_notifier(instance),
                                                &external_netdata_changed_callback,
                                                NULL);

    VERIFY_OR_EXIT(error == NS_ERROR_NO_BUFS);

    error = NS_ERROR_NONE;

    notifier_signal(instance_get_notifier(instance), NS_CHANGED_THREAD_NETDATA);

exit:
    if (error != NS_ERROR_NONE) {
        printf("notifier test FAILED\r\n");
    }
    return;
#endif
}

void
test_logging(void)
{
    test_notifier();
    print_logging();
}
