#include "ns/sys/core/common/instance.h"

static NS_DEFINE_ALIGNED_VAR(instance_raw, sizeof(instance_t), uint64_t);

static timer_scheduler_t timer_scheduler(void)
{
    instance_t *inst = instance_get();
    return inst->timer_sched;
}

static tasklet_scheduler_t tasklet_scheduler(void)
{
    instance_t *inst = instance_get();
    return inst->tasklet_sched;
}

instance_t *instance_init(void)
{
    instance_t *inst = instance_get();

    VERIFY_OR_EXIT(inst->is_initialized == false);

    // --- Timer
    timer_scheduler_make_new(inst);

    // --- Tasklet
    inst->tasklet_sched.head = NULL;
    inst->tasklet_sched.tail = NULL;

    // --- Message
    message_pool_make_new(inst);

    // --- Instance API
    inst->get_timer_scheduler = timer_scheduler;
    inst->get_tasklet_scheduler = tasklet_scheduler;

    inst->is_initialized = true;
exit:
    return inst;
}

instance_t *instance_get(void)
{
    void *inst = &instance_raw;
    return (instance_t *)inst;
}
