#ifndef NS_CORE_COMMON_TASKLET_H_
#define NS_CORE_COMMON_TASKLET_H_

#include "ns/include/error.h"

typedef struct _tasklet tasklet_t;
typedef void (*tasklet_handler_t)(tasklet_t *taklet);

struct _tasklet {
    tasklet_handler_t handler;
    tasklet_t *next;
};

typedef struct _tasklet_scheduler {
    tasklet_t *head;
    tasklet_t *tail;
} tasklet_scheduler_t;

ns_error_t
tasklet_post(void *instance, tasklet_t *tasklet);

bool
tasklet_are_pending(void *instance);

void 
tasklet_process_queued_task(void *instance);

#endif // NS_CORE_COMMON_TASKLET_H_
