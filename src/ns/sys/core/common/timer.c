#include "ns/include/nstd.h"
#include "ns/include/platform/alarm.h"
#include "ns/sys/core/common/instance.h"

static bool is_strictly_before(uint32_t time_a, uint32_t time_b);
static bool does_fire_before(timer_t *timer, timer_t *cur, uint32_t now);
static void alarm_set(void *instance);
static void alarm_start_at(uint32_t t0, uint32_t dt);
static void alarm_stop(void);
static uint32_t alarm_get_now(void);

static void timer_add(void *instance, timer_t *timer);
static void timer_remove(void *instance,  timer_t *timer);
static void timer_process(void *instance);

void timer_start(void *instance, timer_t *timer, uint32_t t0, uint32_t dt)
{
    ns_assert(dt <= TIMER_MAX_DT);
    timer->firetime = t0 + dt;
    timer_add((instance_t *)instance, timer);
}

static void timer_add(void *instance, timer_t *timer)
{
    timer_remove(instance, timer);
    instance_t *inst = (instance_t *)instance;
    timer_t *head = inst->timer_sched.head;
    if (head == NULL) {
        // update timer scheduler head
        inst->timer_sched.head = timer;
        timer->next = NULL;
        alarm_set(inst);
    } else {
        timer_t *prev = NULL;
        timer_t *cur;
        for (cur = head; cur; cur = cur->next) {
            if (does_fire_before(timer, cur, alarm_get_now())) {
                if (prev) {
                    timer->next = cur;
                    prev->next = timer;
                } else {
                    timer->next = head;
                    // update timer scheduler head
                    inst->timer_sched.head = timer;
                    alarm_set(inst);
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

static void timer_remove(void *instance, timer_t *timer)
{
    VERIFY_OR_EXIT(timer->next != timer);
    instance_t *inst = (instance_t *)instance;
    timer_t *head = inst->timer_sched.head;
    if (head == timer) {
        // update timer scheduler head
        inst->timer_sched.head = timer->next;
        alarm_set(inst);
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

static void timer_process(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    timer_t *timer = inst->timer_sched.head;
    if (timer) {
        if (!is_strictly_before(alarm_get_now(), timer->firetime)) {
            timer_remove(inst, timer);
            timer->handler(timer);
        } else {
            alarm_set(inst);
        }
    } else {
        alarm_set(inst);
    }
}

static bool is_strictly_before(uint32_t time_a, uint32_t time_b)
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

static bool does_fire_before(timer_t *timer, timer_t *cur, uint32_t now)
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

static void alarm_set(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    timer_t *head = inst->timer_sched.head;
    if (head == NULL) {
        alarm_stop();
    } else {
        uint32_t now = alarm_get_now();
        uint32_t remaining = is_strictly_before(now, head->firetime) ? (head->firetime - now) : 0;
        alarm_start_at(now, remaining);
    }
}

static void alarm_start_at(uint32_t t0, uint32_t dt)
{
    ns_plat_alarm_start_at(t0, dt);
}

static void alarm_stop(void)
{
    ns_plat_alarm_stop();
}

static uint32_t alarm_get_now(void)
{
    return ns_plat_alarm_get_now();
}

void ns_plat_alarm_fired(ns_instance_t instance)
{
    void *inst = (void *)instance;
    VERIFY_OR_EXIT(instance_is_initialized(inst));
    timer_process(inst);
exit:
    return;
}
