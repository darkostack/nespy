#ifndef NS_CORE_COMMON_TASKLET_H_
#define NS_CORE_COMMON_TASKLET_H_

#include "ns/sys/core/common/instance.h"

typedef void (*tasklet_handler_t)(tasklet_t *taklet);

struct _tasklet {
    tasklet_handler_t handler;
    tasklet_t *next;
};

struct _tasklet_scheduler {
    tasklet_t *head;
    tasklet_t *tail;
};

ns_error_t tasklet_post(instance_t *instance, tasklet_t *tasklet);
bool tasklet_are_pending(instance_t *instance);
void tasklet_process_queued_task(instance_t *instance);

#endif // NS_CORE_COMMON_TASKLET_H_
