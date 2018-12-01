#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/trickle_timer.h"

// --- private functions declarations
static void
trickle_handle_timer(trickle_timer_t *trickle_timer);

static void
trickle_handle_end_of_time_in_interval(trickle_timer_t *trickle_timer);

static void
trickle_handle_end_of_interval(trickle_timer_t *trickle_timer);

static void
trickle_handle_timer(timer_t *timer);

void
trickle_timer_ctor(void *instance,
                   trickle_timer_t *trickle_timer,
                   trickle_timer_handler_t *transmit_handler,
                   trickle_timer_handler_t *interval_expired_handler)
{
    ns_assert(transmit_handler != NULL);

    timer_milli_ctor(instance, &trickle_timer->timer, &trickle_handle_timer);

    trickle_timer->interval_min = 0;
    trickle_timer->interval_max = 0;
    trickle_timer->interval = 0;
    trickle_timer->time_in_interval = 0;
    trickle_timer->transmit_handler = transmit_handler;
    trickle_timer->interval_expired_handler = interval_expired_handler;
    trickle_timer->mode = TRICKLE_TIMER_MODE_NORMAL;
    trickle_timer->is_running = false;
    trickle_timer->in_transmit_phase = false;
}

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


