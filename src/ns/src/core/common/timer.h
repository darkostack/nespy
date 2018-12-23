#ifndef NS_CORE_COMMON_TIMER_H_
#define NS_CORE_COMMON_TIMER_H_

#include "core/core-config.h"

enum {
    TIMER_MAX_DT = (1UL << 31) - 1,
};

typedef struct _timer timer_t;
typedef timer_t timer_milli_t;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
typedef timer_t timer_micro_t;
#endif

typedef void (*timer_handler_func_t)(void *timer);
typedef struct _timer_handler {
    timer_handler_func_t func;
    void *context;
} timer_handler_t;

struct _timer {
    void *instance;
    timer_handler_t handler;
    uint32_t firetime;
    timer_t *next;
};

typedef struct _timer_scheduler timer_scheduler_t;
struct _timer_scheduler {
    timer_t *head;
};

typedef struct _alarm_api alarm_api_t;
struct _alarm_api {
    void (*alarm_start_at)(void *instance, uint32_t t0, uint32_t dt);
    void (*alarm_stop)(void *instance);
    uint32_t (*alarm_get_now)(void);
};

void
timer_scheduler_ctor(timer_scheduler_t *timer_scheduler);

bool
timer_is_running(void *timer);

uint32_t
timer_get_firetime(void *timer);

void
timer_ctor(void *instance, void *timer, timer_handler_func_t handler, void *context);

// --- milliseconds timer
void
timer_milli_start(timer_milli_t *milli, uint32_t dt);

void
timer_milli_start_at(timer_milli_t *milli, uint32_t t0, uint32_t dt);

void
timer_milli_stop(timer_milli_t *milli);

// --- microseconds timer
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
void
timer_micro_start(timer_micro_t *micro, uint32_t dt);

void
timer_micro_start_at(timer_micro_t *micro, uint32_t t0, uint32_t dt);

void
timer_micro_stop(timer_micro_t *micro);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

#endif // NS_CORE_COMMON_TIMER_H_
