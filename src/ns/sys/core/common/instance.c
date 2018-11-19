#include "ns/sys/core/common/instance.h"

static NS_DEFINE_ALIGNED_VAR(instance_raw, sizeof(instance_t), uint64_t);

instance_t *instance_init(void)
{
    instance_t *inst = instance_get();
    VERIFY_OR_EXIT(inst->is_initialized == false);
    // --- Timer
    inst->timer_sched.head = NULL;
    // --- Tasklet
    inst->tasklet_sched.head = NULL;
    inst->tasklet_sched.tail = NULL;

    inst->is_initialized = true;
exit:
    return inst;
}

instance_t *instance_get(void)
{
    void *inst = &instance_raw;
    return (instance_t *)inst;
}

bool instance_is_initialized(instance_t *instance)
{
    return instance->is_initialized;
}
