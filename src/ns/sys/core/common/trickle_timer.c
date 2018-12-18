#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/debug.h"
#include "ns/sys/core/common/random.h"
#include "ns/sys/core/common/trickle_timer.h"

// --- private functions declarations
static void
trickle_timer_start_new_interval(trickle_timer_t *trickle_timer);

static void
trickle_timer_handle_timer(void *timer);

static void
trickle_timer_handle_end_of_time_in_interval(trickle_timer_t *trickle_timer);

static void
trickle_timer_handle_end_of_interval(trickle_timer_t *trickle_timer);

// --- trickle timer functions
void
trickle_timer_ctor(void *instance,
                   trickle_timer_t *trickle_timer,
                   trickle_timer_handler_func_t transmit_handler,
                   trickle_timer_handler_func_t interval_expired_handler)
{
    assert(transmit_handler != NULL);

    timer_milli_ctor(instance, &trickle_timer->milli, &trickle_timer_handle_timer, (void *)trickle_timer);

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
trickle_timer_is_running(trickle_timer_t *trickle_timer)
{
    return trickle_timer->is_running;
}

ns_error_t
trickle_timer_start(trickle_timer_t *trickle_timer,
                    uint32_t interval_min,
                    uint32_t interval_max,
                    trickle_timer_mode_t mode)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(interval_max >= interval_min, error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(interval_min != 0 || interval_max != 0, error =  NS_ERROR_INVALID_ARGS);

    trickle_timer->interval_min = interval_min;
    trickle_timer->interval_max = interval_max;
    trickle_timer->mode = mode;
    trickle_timer->is_running = true;

    // select interval randomly from range (min - max)
    trickle_timer->interval = random_get_uint32_in_range(trickle_timer->interval_min,
                                                         trickle_timer->interval_max + 1);

    trickle_timer_start_new_interval(trickle_timer);

exit:
    return error;
}

void
trickle_timer_stop(trickle_timer_t *trickle_timer)
{
    trickle_timer->is_running = false;
    timer_milli_stop(&trickle_timer->milli);
}

void
trickle_timer_indicate_inconsistent(trickle_timer_t *trickle_timer)
{
    // if interval is equal to minimum when an `inconsistent` event is received,
    // do nothing
    VERIFY_OR_EXIT(trickle_timer->is_running && (trickle_timer->interval != trickle_timer->interval_min));

    trickle_timer->interval = trickle_timer->interval_min;
    trickle_timer_start_new_interval(trickle_timer);

exit:
    return;
}

// --- private functions

static void
trickle_timer_start_new_interval(trickle_timer_t *trickle_timer)
{
    uint32_t half_interval;
    trickle_timer->in_transmit_phase = true;

    switch (trickle_timer->mode) {
    case TRICKLE_TIMER_MODE_NORMAL:
        half_interval = trickle_timer->interval / 2;
        VERIFY_OR_EXIT(half_interval < trickle_timer->interval, trickle_timer->interval = half_interval);
        // select random point in the interval taken from the range (I/2, I)
        trickle_timer->time_in_interval = random_get_uint32_in_range(half_interval, trickle_timer->interval);
        break;
    case TRICKLE_TIMER_MODE_PLAIN_TIMER:
        trickle_timer->time_in_interval = trickle_timer->interval;
        break;
    case TRICKLE_TIMER_MODE_MPL:
        trickle_timer->time_in_interval = random_get_uint32_in_range(0, trickle_timer->interval + 1);
        break;
    }

exit:
    timer_milli_start(&trickle_timer->milli, trickle_timer->time_in_interval);
}

static void
trickle_timer_handle_timer(void *timer)
{
    trickle_timer_t *trickle_timer = (trickle_timer_t *)((timer_t *)timer)->handler.arg;
    if (trickle_timer->in_transmit_phase) {
        trickle_timer_handle_end_of_time_in_interval(trickle_timer);
    } else {
        trickle_timer_handle_end_of_interval(trickle_timer);
    }
}

static void
trickle_timer_handle_end_of_time_in_interval(trickle_timer_t *trickle_timer)
{
    bool should_continue = trickle_timer->transmit_handler(trickle_timer);
    VERIFY_OR_EXIT(should_continue, timer_milli_stop(&trickle_timer->milli));

    switch (trickle_timer->mode) {
    case TRICKLE_TIMER_MODE_PLAIN_TIMER:
        // select random interval in (Imin, Imax) and restart
        trickle_timer->interval = random_get_uint32_in_range(trickle_timer->interval_min,
                                                             trickle_timer->interval_max + 1);
        trickle_timer_start_new_interval(trickle_timer);
        break;
    case TRICKLE_TIMER_MODE_NORMAL:
    case TRICKLE_TIMER_MODE_MPL:
        // waiting for the rest of the interval to elapse
        trickle_timer->in_transmit_phase = false;
        timer_milli_start(&trickle_timer->milli, trickle_timer->interval - trickle_timer->time_in_interval);
        break;
    }

exit:
    return;
}

static void
trickle_timer_handle_end_of_interval(trickle_timer_t *trickle_timer)
{
    // double the interval and ensure result is below max
    if (trickle_timer->interval == 0) {
        trickle_timer->interval = 1;
    } else if (trickle_timer->interval <= trickle_timer->interval_max - trickle_timer->interval) {
        trickle_timer->interval *= 2;
    } else {
        trickle_timer->interval = trickle_timer->interval_max;
    }

    if (trickle_timer->interval_expired_handler) {
        bool should_continue = trickle_timer->interval_expired_handler(trickle_timer);
        VERIFY_OR_EXIT(should_continue, timer_milli_stop(&trickle_timer->milli));
    }

    trickle_timer_start_new_interval(trickle_timer);

exit:
    return;
}
