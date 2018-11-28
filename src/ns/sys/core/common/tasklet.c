#include "ns/include/tasklet.h"
#include "ns/sys/core/common/instance.h"

// --- private functions declarations
static tasklet_t *
tasklet_pop(instance_t *instance);

// --- tasklet scheduler functions
void
tasklet_scheduler_ctor(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    inst->tasklet_sched.head = NULL;
    inst->tasklet_sched.tail = NULL;
}

ns_error_t
tasklet_post(tasklet_t *tasklet)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    tasklet_t *tail = inst->tasklet_sched.tail;
    VERIFY_OR_EXIT(tail != tasklet && tasklet->next == NULL,
                   error = NS_ERROR_ALREADY);
    if (tail == NULL) {
        inst->tasklet_sched.head = tasklet; 
        inst->tasklet_sched.tail = tasklet;
        ns_tasklet_signal_pending((void *)inst);
    } else {
        inst->tasklet_sched.tail->next = tasklet;
        inst->tasklet_sched.tail = tasklet;
    }
exit:
    return error;
}

bool
tasklet_are_pending(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    return inst->tasklet_sched.head != NULL;
}

void
tasklet_process_queued_task(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    tasklet_t *tail = inst->tasklet_sched.tail;
    tasklet_t *cur;
    while ((cur = tasklet_pop(inst)) != NULL) {
        cur->handler(cur);
        // only process tasklets that were queued at the time this method was
        // called
        if (cur == tail) {
            if (inst->tasklet_sched.head != NULL) {
                ns_tasklet_signal_pending((void *)inst);
            }
            break;
        }
    }
}

// --- private functions
static tasklet_t *
tasklet_pop(instance_t *instance)
{
    tasklet_t *task = instance->tasklet_sched.head;
    if (task != NULL) {
        instance->tasklet_sched.head = instance->tasklet_sched.head->next;
        if (instance->tasklet_sched.head == NULL) {
            instance->tasklet_sched.tail = NULL;
        }
        task->next = NULL;
    }
    return task;
}
