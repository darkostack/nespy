#include "ns/include/tasklet.h"
#include "ns/sys/core/common/tasklet.h"

tasklet_scheduler_t tasklet_scheduler_obj;

static tasklet_t *pop_tasklet(instance_t *instance);

ns_error_t tasklet_post(instance_t *instance, tasklet_t *tasklet)
{
    ns_error_t error = NS_ERROR_NONE;
    tasklet_t *tail = instance->tasklet_sched->tail;
    if (tail == tasklet && tasklet->next != NULL) {
        error = NS_ERROR_ALREADY;
        goto exit;
    }
    if (tail == NULL) {
        instance->tasklet_sched->head = tasklet; 
        instance->tasklet_sched->tail = tasklet;
        ns_tasklet_signal_pending(instance);
    } else {
        instance->tasklet_sched->tail->next = tasklet;
        instance->tasklet_sched->tail = tasklet;
    }
exit:
    return error;
}

bool tasklet_are_pending(instance_t *instance)
{
    return instance->tasklet_sched->head != NULL;
}

void tasklet_process_queued_task(instance_t *instance)
{
    tasklet_t *tail = instance->tasklet_sched->tail;
    tasklet_t *cur;
    while ((cur = pop_tasklet(instance)) != NULL) {
        cur->handler(cur);
        // only process tasklets that were queued at the time this method was
        // called
        if (cur == tail) {
            if (instance->tasklet_sched->head != NULL) {
                ns_tasklet_signal_pending(instance);
            }
            break;
        }
    }
}

static tasklet_t *pop_tasklet(instance_t *instance)
{
    tasklet_t *task = instance->tasklet_sched->head;
    if (task != NULL) {
        instance->tasklet_sched->head = instance->tasklet_sched->head->next;
        if (instance->tasklet_sched->head == NULL) {
            instance->tasklet_sched->tail = NULL;
        }
        task->next = NULL;
    }
    return task;
}
