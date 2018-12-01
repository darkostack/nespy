#ifndef NS_CORE_COMMON_TIMER_H_
#define NS_CORE_COMMON_TIMER_H_

#include "ns/sys/core/core-config.h"

enum {
    TIMER_MAX_DT = (1UL << 31) - 1,
};

typedef struct _timer timer_t;
typedef struct _timer_scheduler timer_scheduler_t;
typedef struct _alarm_api alarm_api_t;

typedef void (*timer_handler_t)(timer_t *timer);

struct _timer {
    void *instance;
    timer_handler_t handler;
    void *handler_arg;
    uint32_t firetime;
    timer_t *next;
};

struct _timer_scheduler {
    timer_t *head;
};

struct _alarm_api {
    void (*alarm_start_at)(void *instance, uint32_t t0, uint32_t dt);
    void (*alarm_stop)(void *instance);
    uint32_t (*alarm_get_now)(void);
};

void
timer_scheduler_ctor(timer_scheduler_t *timer_scheduler);

void
timer_milli_ctor(void *instance, timer_t *timer, timer_handler_t handler, void *handler_arg);

void
timer_milli_start(timer_t *timer, uint32_t dt);

void
timer_milli_start_at(timer_t *timer, uint32_t t0, uint32_t dt);

void
timer_milli_stop(timer_t *timer);

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
void
timer_micro_ctor(void *instance, timer_t *timer, timer_handler_t handler, void *handler_arg);

void
timer_micro_start(timer_t *timer, uint32_t dt);

void
timer_micro_start_at(timer_t *timer, uint32_t t0, uint32_t dt);

void
timer_micro_stop(timer_t *timer);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

#endif // NS_CORE_COMMON_TIMER_H_
