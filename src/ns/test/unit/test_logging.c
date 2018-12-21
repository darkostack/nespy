#include "ns/sys/core/core-config.h"
#include "ns/include/platform/random.h"
#include "ns/sys/core/common/instance.h"

#define PRINT_LOGGING 1

static void
print_logging(void)
{
#if PRINT_LOGGING
    ns_log_crit_api("print test 0x%08x", 0xdeadbeef);

    uint8_t dump_buffer[128];

    for (unsigned i = 0; i < sizeof(dump_buffer); i++)
    {
        dump_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    ns_dump_crit_mac("test buffer", dump_buffer, sizeof(dump_buffer));
#endif
}

void
channel_changed_callback(notifier_callback_t *callback, ns_changed_flags_t flags)
{
    VERIFY_OR_EXIT(flags & NS_CHANGED_THREAD_CHANNEL);

    printf("notifier: CHANGED THREAD CHANNEL\r\n");

exit:
    return;
}

static void
test_notifier(void)
{
    instance_t *instance = instance_get();
    // signal if first should only print one time
    notifier_signal_if_first(instance_get_notifier(instance), NS_CHANGED_THREAD_PANID);
    notifier_signal_if_first(instance_get_notifier(instance), NS_CHANGED_THREAD_PANID);
    notifier_signal_if_first(instance_get_notifier(instance), NS_CHANGED_THREAD_PANID);

    // signal various flags
    notifier_signal(instance_get_notifier(instance), NS_CHANGED_THREAD_CHILD_ADDED);

    //notifier_callback_t channel_changed;
    //notifier_callback_ctor(&channel_changed, &channel_changed_callback);
    //notifier_register_callback(instance_get_notifier(instance), &channel_changed);

    //notifier_signal(instance_get_notifier(instance), NS_CHANGED_THREAD_CHANNEL);
}

void
test_logging(void)
{
    test_notifier();
    print_logging();
}
