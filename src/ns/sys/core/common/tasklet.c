#include "ns/include/tasklet.h"
#include "ns/sys/core/common/instance.h"

// --- tasklet scheduler functions
static ns_error_t tasklet_post(tasklet_t *tasklet);
static bool tasklet_are_pending(void);
static void tasklet_process_queued_task(void);

// --- private functions
static tasklet_t *tasklet_pop(instance_t *instance);

void tasklet_scheduler_make_new(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    inst->tasklet_sched.head = NULL;
    inst->tasklet_sched.tail = NULL;
    // --- tasklet scheduler functions
    inst->tasklet_sched.post = tasklet_post;
    inst->tasklet_sched.are_pending = tasklet_are_pending;
    inst->tasklet_sched.process_queued_task = tasklet_process_queued_task;
}

// --- tasklet scheduler functions
static ns_error_t tasklet_post(tasklet_t *tasklet)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *instance = instance_get();
    tasklet_t *tail = instance->tasklet_sched.tail;
    VERIFY_OR_EXIT(tail != tasklet && tasklet->next == NULL,
                   error = NS_ERROR_ALREADY);
    if (tail == NULL) {
        instance->tasklet_sched.head = tasklet; 
        instance->tasklet_sched.tail = tasklet;
        ns_tasklet_signal_pending((void *)instance);
    } else {
        instance->tasklet_sched.tail->next = tasklet;
        instance->tasklet_sched.tail = tasklet;
    }
exit:
    return error;
}

static bool tasklet_are_pending(void)
{
    instance_t *instance = instance_get();
    return instance->tasklet_sched.head != NULL;
}

static void tasklet_process_queued_task(void)
{
    instance_t *instance = instance_get();
    tasklet_t *tail = instance->tasklet_sched.tail;
    tasklet_t *cur;
    while ((cur = tasklet_pop(instance)) != NULL) {
        cur->handler(cur);
        // only process tasklets that were queued at the time this method was
        // called
        if (cur == tail) {
            if (instance->tasklet_sched.head != NULL) {
                ns_tasklet_signal_pending((void *)instance);
            }
            break;
        }
    }
}

// --- private functions
static tasklet_t *tasklet_pop(instance_t *instance)
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
