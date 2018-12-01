#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"

static NS_DEFINE_ALIGNED_VAR(instance_raw, sizeof(instance_t), uint64_t);

// --- instance functions
instance_t *
instance_ctor(void)
{
    instance_t *inst = instance_get();

    VERIFY_OR_EXIT(inst->is_initialized == false);

    // objects constructor
    timer_scheduler_ctor(&inst->timer_milli_scheduler);
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_scheduler_ctor(&inst->timer_micro_scheduler);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    tasklet_scheduler_ctor(&inst->tasklet_scheduler);
    message_pool_ctor(&inst->message_pool);
    heap_ctor(&inst->heap);

    inst->is_initialized = true;

    extern ns_error_t test_message_write_read(void *instance);
    test_message_write_read((void *)inst);

    extern ns_error_t test_message_utility(void *instance);
    test_message_utility((void *)inst);

    extern ns_error_t test_message_queue(void *instance);
    test_message_queue((void *)inst);

    extern ns_error_t test_message_priority_queue(void *instance);
    test_message_priority_queue((void *)inst);

    extern ns_error_t test_heap_allocate_single(void *instance);
    test_heap_allocate_single((void *)inst);

    extern ns_error_t test_heap_allocate_multiple(void *instance);
    test_heap_allocate_multiple((void *)inst);

    extern ns_error_t test_string(void);
    test_string();

    extern ns_error_t test_mac_header(void);
    test_mac_header();

    extern ns_error_t test_mac_channel_mask(void);
    test_mac_channel_mask();

exit:
    return inst;
}

instance_t *
instance_get(void)
{
    void *inst = &instance_raw;
    return (instance_t *)inst;
}
