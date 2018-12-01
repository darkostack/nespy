#include "ns/include/nstd.h"
#include "ns/include/platform/alarm-milli.h"
#include "ns/include/platform/alarm-micro.h"
#include "ns/sys/core/common/instance.h"

const alarm_api_t alarm_milli_api = {
    &ns_plat_alarm_milli_start_at,
    &ns_plat_alarm_milli_stop,
    &ns_plat_alarm_milli_get_now,
};

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
const alarm_api_t alarm_micro_api = {
    &ns_plat_alarm_micro_start_at,
    &ns_plat_alarm_micro_stop,
    &ns_plat_alarm_micro_get_now,
};
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

// --- private functions declarations
static void
timer_add(timer_scheduler_t *timer_scheduler, timer_t *timer, const alarm_api_t *alarm_api);

static void
timer_remove(timer_scheduler_t *timer_scheduler,  timer_t *timer, const alarm_api_t *alarm_api);

static void
timer_process(timer_scheduler_t *timer_scheduler, const alarm_api_t *alarm_api);

static bool
is_strictly_before(uint32_t time_a, uint32_t time_b);

static bool
does_fire_before(timer_t *timer, timer_t *cur, uint32_t now);

static void
alarm_set(timer_scheduler_t *timer_scheduler, const alarm_api_t *alarm_api);

// --- timer functions
void
timer_scheduler_ctor(timer_scheduler_t *timer_scheduler)
{
    timer_scheduler->head = NULL;
}

void
timer_milli_ctor(void *instance, timer_t *timer, timer_handler_t handler)
{
    ns_assert(handler != NULL);
    timer->instance = instance;
    timer->handler = handler;
    timer->firetime = 0;
    timer->next = NULL;
}

void
timer_milli_start(timer_t *timer, uint32_t dt)
{
    ns_assert(dt <= TIMER_MAX_DT);
    timer->firetime = ns_plat_alarm_milli_get_now() + dt;
    timer_add(&((instance_t *)timer->instance)->timer_milli_scheduler, timer, &alarm_milli_api);
}

void
timer_milli_start_at(timer_t *timer, uint32_t t0, uint32_t dt)
{
    ns_assert(dt <= TIMER_MAX_DT);
    timer->firetime = t0 + dt;
    timer_add(&((instance_t *)timer->instance)->timer_milli_scheduler, timer, &alarm_milli_api);
}

void
timer_milli_stop(timer_t *timer)
{
    timer_remove(&((instance_t *)timer->instance)->timer_milli_scheduler, timer, &alarm_milli_api);
}

void
ns_plat_alarm_milli_fired(ns_instance_t instance)
{
    VERIFY_OR_EXIT(((instance_t *)instance)->is_initialized);
    timer_process(&((instance_t *)instance)->timer_milli_scheduler, &alarm_milli_api);
exit:
    return;
}

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
void
timer_micro_ctor(void *instance, timer_t *timer, timer_handler_t handler)
{
    ns_assert(handler != NULL);
    timer->instance = instance;
    timer->handler = handler;
    timer->firetime = 0;
    timer->next = NULL;
}

void
timer_micro_start(timer_t *timer, uint32_t dt)
{
    ns_assert(dt <= TIMER_MAX_DT);
    timer->firetime = ns_plat_alarm_micro_get_now() + dt;
    timer_add(&((instance_t *)timer->instance)->timer_micro_scheduler, timer, &alarm_micro_api);
}

void
timer_micro_start_at(timer_t *timer, uint32_t t0, uint32_t dt)
{
    ns_assert(dt <= TIMER_MAX_DT);
    timer->firetime = t0 + dt;
    timer_add(&((instance_t *)timer->instance)->timer_micro_scheduler, timer, &alarm_micro_api);
}

void
timer_micro_stop(timer_t *timer)
{
    timer_remove(&((instance_t *)timer->instance)->timer_micro_scheduler, timer, &alarm_micro_api);
}

void
ns_plat_alarm_micro_fired(ns_instance_t instance)
{
    VERIFY_OR_EXIT(((instance_t *)instance)->is_initialized);
    timer_process(&((instance_t *)instance)->timer_micro_scheduler, &alarm_micro_api);
exit:
    return;
}
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

// --- private functions
static void
timer_add(timer_scheduler_t *timer_scheduler, timer_t *timer, const alarm_api_t *alarm_api)
{
    timer_remove(timer_scheduler, timer, alarm_api);
    timer_t *head = timer_scheduler->head;
    if (head == NULL) {
        // update timer scheduler head
        timer_scheduler->head = timer;
        timer->next = NULL;
        alarm_set(timer_scheduler, alarm_api);
    } else {
        timer_t *prev = NULL;
        timer_t *cur;
        for (cur = head; cur; cur = cur->next) {
            if (does_fire_before(timer, cur, alarm_api->alarm_get_now())) {
                if (prev) {
                    timer->next = cur;
                    prev->next = timer;
                } else {
                    timer->next = head;
                    // update timer scheduler head
                    timer_scheduler->head = timer;
                    alarm_set(timer_scheduler, alarm_api);
                }
                break;
            }
            prev = cur;
        }
        if (cur == NULL) {
            prev->next = timer;
            timer->next = NULL;
        }
    }
}

static void
timer_remove(timer_scheduler_t *timer_scheduler, timer_t *timer, const alarm_api_t *alarm_api)
{
    VERIFY_OR_EXIT(timer->next != timer);
    timer_t *head = timer_scheduler->head;
    if (head == timer) {
        // update timer scheduler head
        timer_scheduler->head = timer->next;
        alarm_set(timer_scheduler, alarm_api);
    } else {
        for (timer_t *cur = head; cur; cur = cur->next) {
            if (cur->next == timer) {
                cur->next = timer->next;
                break;
            }
        }
    }
    timer->next = timer;
exit:
    return;
}

static void
timer_process(timer_scheduler_t *timer_scheduler, const alarm_api_t *alarm_api)
{
    timer_t *timer = timer_scheduler->head;
    if (timer) {
        if (!is_strictly_before(alarm_api->alarm_get_now(), timer->firetime)) {
            timer_remove(timer_scheduler, timer, alarm_api);
            timer->handler(timer);
        } else {
            alarm_set(timer_scheduler, alarm_api);
        }
    } else {
        alarm_set(timer_scheduler, alarm_api);
    }
}

static bool
is_strictly_before(uint32_t time_a, uint32_t time_b)
{
    uint32_t diff = time_a - time_b;

    // Three cases:
    // 1) aTimeA is before  aTimeB  => Difference is negative
    //                                 (last bit of difference is set)
    //                              => Returning true.
    //
    // 2) aTimeA is same as aTimeB  => Difference is zero
    //                                 (last bit of difference is clear)
    //                              => Returning false.
    //
    // 3) aTimeA is after   aTimeB  => Difference is positive
    //                                 (last bit of difference is clear)
    //                              => Returning false.

    return ((diff & (1UL << 31)) != 0);
}

static bool
does_fire_before(timer_t *timer, timer_t *cur, uint32_t now)
{
    bool retval;
    bool is_before_now = is_strictly_before(timer->firetime, now);

    // Check if one timer is before `now` and the other one is not.
    if (is_strictly_before(cur->firetime, now) != is_before_now) {
        // One timer is before `now` and the other one is not,
        // so if this timer's fire time is before `now` then
        // the second fire time would be after `now` and
        // this timer would fire before the second timer.
        retval = is_before_now;
    } else {
        // Both timers are before `now` or both are after `now`.
        // Either way the difference is guaranteed to be less
        // than `TIMER_MAX_DT` so we can safely compare the fire times directly.
        retval = is_strictly_before(timer->firetime, cur->firetime);
    }

    return retval;
}

static void
alarm_set(timer_scheduler_t *timer_scheduler, const alarm_api_t *alarm_api)
{
    timer_t *head = timer_scheduler->head;
    if (head == NULL) {
        alarm_api->alarm_stop((void *)instance_get());
    } else {
        uint32_t now = alarm_api->alarm_get_now();
        uint32_t remaining = is_strictly_before(now, head->firetime) ? (head->firetime - now) : 0;
        alarm_api->alarm_start_at((void *)instance_get(), now, remaining);
    }
}
