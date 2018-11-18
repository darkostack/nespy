#include "ns/include/tasklet.h"
#include "ns/sys/core/common/tasklet.h"

void ns_tasklet_process(ns_instance_t instance)
{
    instance_t *inst = (instance_t *)instance;
    if (!inst->is_initialized) {
        goto exit;
    }
    tasklet_process_queued_task(inst);
exit:
    return;
}

bool ns_tasklet_are_pending(ns_instance_t instance)
{
    bool retval = false;
    instance_t *inst = (instance_t *)instance;
    if (!inst->is_initialized) {
        goto exit;
    }
    retval = tasklet_are_pending(instance);
exit:
    return retval;
}

__attribute__((weak)) void ns_tasklet_signal_pending(ns_instance_t instance)
{
    // incase nobody define this function
}


