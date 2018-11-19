#include "ns/include/tasklet.h"
#include "ns/sys/core/common/instance.h"

void ns_tasklet_process(ns_instance_t instance)
{
    instance_t *inst = (instance_t *)instance;
    VERIFY_OR_EXIT(instance_is_initialized(inst));
    tasklet_process_queued_task(inst);
exit:
    return;
}

bool ns_tasklet_are_pending(ns_instance_t instance)
{
    bool retval = false;
    instance_t *inst = (instance_t *)instance;
    VERIFY_OR_EXIT(instance_is_initialized(inst));
    retval = tasklet_are_pending(instance);
exit:
    return retval;
}

__attribute__((weak)) void ns_tasklet_signal_pending(ns_instance_t instance)
{
}
