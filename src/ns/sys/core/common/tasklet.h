#ifndef NS_CORE_COMMON_TASKLET_H_
#define NS_CORE_COMMON_TASKLET_H_

#include "ns/include/error.h"

typedef struct _tasklet tasklet_t;

typedef void (*tasklet_handler_func_t)(void *tasklet);
typedef struct _tasklet_handler {
    tasklet_handler_func_t func;
    void *arg;
} tasklet_handler_t;

struct _tasklet {
    void *instance;
    tasklet_handler_t handler;
    tasklet_t *next;
};

typedef struct _tasklet_scheduler {
    tasklet_t *head;
    tasklet_t *tail;
} tasklet_scheduler_t;

void
tasklet_scheduler_ctor(tasklet_scheduler_t *tasklet_scheduler);

void
tasklet_ctor(void *instance, tasklet_t *tasklet, tasklet_handler_func_t handler, void *handler_arg);

ns_error_t
tasklet_post(tasklet_t *tasklet);

bool
tasklet_are_pending(void *instance);

void 
tasklet_process_queued_task(void *instance);

#endif // NS_CORE_COMMON_TASKLET_H_
