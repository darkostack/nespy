#ifndef NS_CORE_COMMON_TIMER_H_
#define NS_CORE_COMMON_TIMER_H_

#include "ns/sys/core/common/instance.h"

enum {
    TIMER_MAX_DT = (1UL << 31) - 1,
};

typedef void (*timer_handler_t)(timer_t *timer);

struct _timer {
    timer_handler_t handler;
    uint32_t firetime;
    timer_t *next;
};

struct _timer_scheduler {
    timer_t *head;
};

void timer_start(instance_t *instance, timer_t *timer, uint32_t t0, uint32_t dt);

#endif // NS_CORE_COMMON_TIMER_H_
