#include "ns/sys/core/common/instance.h"

static NS_DEFINE_ALIGNED_VAR(instance_raw, sizeof(instance_t), uint64_t);

// --- instance get functions
static timer_scheduler_t get_timer_scheduler(void);
static tasklet_scheduler_t get_tasklet_scheduler(void);
static message_pool_t get_message_pool(void);

instance_t *instance_init(void)
{
    instance_t *inst = instance_get();

    VERIFY_OR_EXIT(inst->is_initialized == false);

    // --- Timer
    timer_scheduler_make_new(inst);

    // --- Tasklet
    tasklet_scheduler_make_new(inst);

    // --- Message
    message_pool_make_new(inst);

    // --- Instance get functions
    inst->get_timer_scheduler = get_timer_scheduler;
    inst->get_tasklet_scheduler = get_tasklet_scheduler;
    inst->get_message_pool = get_message_pool;

    inst->is_initialized = true;
exit:
    return inst;
}

instance_t *instance_get(void)
{
    void *inst = &instance_raw;
    return (instance_t *)inst;
}

// --- instance get functions
static timer_scheduler_t get_timer_scheduler(void)
{
    instance_t *inst = instance_get();
    return inst->timer_sched;
}

static tasklet_scheduler_t get_tasklet_scheduler(void)
{
    instance_t *inst = instance_get();
    return inst->tasklet_sched;
}

static message_pool_t get_message_pool(void)
{
    instance_t *inst = instance_get();
    return inst->message_pool;
}
