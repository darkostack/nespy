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
    // --- tasklet scheduler functions
    ns_error_t (*post)(tasklet_t *tasklet);
    bool (*are_pending)(void);
    void (*process_queued_task)(void);
} tasklet_scheduler_t;

void tasklet_scheduler_make_new(void *instance);

#endif // NS_CORE_COMMON_TASKLET_H_
