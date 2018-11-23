#include "ns/sys/core/common/instance.h"

static NS_DEFINE_ALIGNED_VAR(instance_raw, sizeof(instance_t), uint64_t);

// --- instance functions
instance_t *
instance_init(void)
{
    instance_t *inst = instance_get();

    VERIFY_OR_EXIT(inst->is_initialized == false);

    timer_scheduler_make_new(inst);
    tasklet_scheduler_make_new(inst);
    message_pool_make_new(inst);

    extern ns_error_t test_message_write_read(void);
    test_message_write_read();

    extern ns_error_t test_message_utility(void);
    test_message_utility();

    extern ns_error_t test_message_queue(void);
    test_message_queue();

    extern ns_error_t test_message_priority_queue(void);
    test_message_priority_queue();

    inst->is_initialized = true;

exit:
    return inst;
}

instance_t *
instance_get(void)
{
    void *inst = &instance_raw;
    return (instance_t *)inst;
}
