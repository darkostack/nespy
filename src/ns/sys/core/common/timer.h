#ifndef NS_CORE_COMMON_TIMER_H_
#define NS_CORE_COMMON_TIMER_H_

enum {
    TIMER_MAX_DT = (1UL << 31) - 1,
};

typedef struct _timer timer_t;
typedef void (*timer_handler_t)(timer_t *timer);

struct _timer {
    timer_handler_t handler;
    uint32_t firetime;
    timer_t *next;
};

typedef struct _timer_scheduler {
    timer_t *head;
    // --- timer scheduler functions
    void (*start)(timer_t *timer, uint32_t t0, uint32_t dt);
} timer_scheduler_t;

void timer_scheduler_make_new(void *instance);

#endif // NS_CORE_COMMON_TIMER_H_
