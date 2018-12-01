#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/include/tasklet.h"
#include "ns/sys/core/common/instance.h"

// --- private functions declarations
static tasklet_t *
tasklet_pop(tasklet_scheduler_t *tasklet_scheduler);

// --- tasklet scheduler functions
void
tasklet_scheduler_ctor(tasklet_scheduler_t *tasklet_scheduler)
{
    tasklet_scheduler->head = NULL;
    tasklet_scheduler->tail = NULL;
}

void
tasklet_ctor(void *instance, tasklet_t *tasklet, tasklet_handler_t handler)
{
    ns_assert(handler != NULL);
    tasklet->instance = instance;
    tasklet->handler = handler;
    tasklet->next = NULL;
}

ns_error_t
tasklet_post(tasklet_t *tasklet)
{
    ns_error_t error = NS_ERROR_NONE;
    tasklet_scheduler_t *tasklet_scheduler = &((instance_t *)tasklet->instance)->tasklet_scheduler;
    tasklet_t *tail = tasklet_scheduler->tail;
    VERIFY_OR_EXIT(tail != tasklet && tasklet->next == NULL,
                   error = NS_ERROR_ALREADY);
    if (tail == NULL) {
        tasklet_scheduler->head = tasklet; 
        tasklet_scheduler->tail = tasklet;
        ns_tasklet_signal_pending((void *)tasklet->instance);
    } else {
        tasklet_scheduler->tail->next = tasklet;
        tasklet_scheduler->tail = tasklet;
    }
exit:
    return error;
}

bool
tasklet_are_pending(void *instance)
{
    return ((instance_t *)instance)->tasklet_scheduler.head != NULL;
}

void
tasklet_process_queued_task(void *instance)
{
    tasklet_scheduler_t *tasklet_scheduler = &((instance_t *)instance)->tasklet_scheduler;
    tasklet_t *tail = tasklet_scheduler->tail;
    tasklet_t *cur;
    while ((cur = tasklet_pop(tasklet_scheduler)) != NULL) {
        cur->handler(cur);
        // only process tasklets that were queued at the time this method was
        // called
        if (cur == tail) {
            if (tasklet_scheduler->head != NULL) {
                ns_tasklet_signal_pending(instance);
            }
            break;
        }
    }
}

// --- private functions
static tasklet_t *
tasklet_pop(tasklet_scheduler_t *tasklet_scheduler)
{
    tasklet_t *task = tasklet_scheduler->head;
    if (task != NULL) {
        tasklet_scheduler->head = tasklet_scheduler->head->next;
        if (tasklet_scheduler->head == NULL) {
            tasklet_scheduler->tail = NULL;
        }
        task->next = NULL;
    }
    return task;
}
