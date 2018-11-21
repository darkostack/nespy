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

    extern void message_write_read_test(void);
    message_write_read_test();

    extern void message_queue_test(void);
    message_queue_test();

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
