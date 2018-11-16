#include "ns/core/common/instance.h"

instance_t instance_obj;
extern timer_scheduler_t timer_scheduler_obj;

instance_t *instance_init(void)
{
    instance_t *inst = &instance_obj;

    if (inst->is_initialized) goto exit;

    inst->timer_sched = &timer_scheduler_obj;
    inst->timer_sched->head = NULL;

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
