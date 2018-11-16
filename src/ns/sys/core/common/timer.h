#ifndef NS_CORE_COMMON_TIMER_H_
#define NS_CORE_COMMON_TIMER_H_

#include "ns/sys/core/common/instance.h"

enum {
    timer_max_dt = (1UL << 31) - 1,
};

typedef void (*timer_handler_t)(void);

struct _timer {
    timer_handler_t handler;
    uint32_t max_dt;
    uint32_t firetime;
    timer_t *next;
};

struct _timer_scheduler {
    timer_t *head;
};

void timer_add(instance_t *instance, timer_t *timer);
void timer_remove(instance_t *instance,  timer_t *timer);
void timer_process(instance_t *instance);

#endif // NS_CORE_COMMON_TIMER_H_
