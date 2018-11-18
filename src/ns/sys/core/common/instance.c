#include "ns/sys/core/common/instance.h"

instance_t instance_obj;
extern timer_scheduler_t timer_scheduler_obj;
extern tasklet_scheduler_t tasklet_scheduler_obj;

instance_t *instance_init(void)
{
    instance_t *inst = &instance_obj;
    if (inst->is_initialized) goto exit;
    // Timer
    inst->timer_sched = &timer_scheduler_obj;
    inst->timer_sched->head = NULL;
    // Tasklet
    inst->tasklet_sched = &tasklet_scheduler_obj;
    inst->tasklet_sched->head = NULL;
    inst->tasklet_sched->tail = NULL;
    // Initialized status 
    inst->is_initialized = true;
exit:
    return inst;
}

instance_t *instance_get(void)
{
    return (instance_t *)&instance_obj;
}

bool instance_is_initialized(instance_t *instance)
{
    return instance->is_initialized;
}
