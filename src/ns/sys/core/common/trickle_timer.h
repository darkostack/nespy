#ifndef NS_CORE_COMMON_TRICKLE_TIMER_H_
#define NS_CORE_COMMON_TRICKLE_TIMER_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/error.h"
#include "ns/sys/core/common/timer.h"

typedef struct _trickle_timer trickle_timer_t;
typedef bool (*trickle_timer_handler_t)(trickle_timer_t *trickle_timer);

typedef enum _trickle_timer_mode {
    TRICKLE_TIMER_MODE_NORMAL,
    TRICKLE_TIMER_MODE_PLAIN_TIMER,
    TRICKLE_TIMER_MODE_MPL,
} trickle_timer_mode_t;

struct _trickle_timer {
    timer_t timer;
    uint32_t interval_min;
    uint32_t interval_max;
    uint32_t interval;
    uint32_t time_in_interval;
    trickle_timer_handler_t transmit_handler;
    trickle_timer_handler_t interval_expired_handler;
    trickle_timer_mode_t mode;
    bool is_running : 1;
    bool in_transmit_phase : 1;
};

void
trickle_timer_ctor(void *instance,
                   trickle_timer_t *trickle_timer,
                   trickle_timer_handler_t transmit_handler,
                   trickle_timer_handler_t interval_expired_handler);

bool
trickle_timer_is_running(trickle_timer_t *trickle_timer);

ns_error_t
trickle_timer_start(trickle_timer_t *trickle_timer,
                    uint32_t interval_min,
                    uint32_t interval_max,
                    trickle_timer_mode_t mode);

void
trickle_timer_stop(trickle_timer_t *trickle_timer);

void
trickle_timer_indicate_inconsistent(trickle_timer_t *trickle_timer);

#endif // NS_CORE_COMMON_TRICKLE_TIMER_H_
