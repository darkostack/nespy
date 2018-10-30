#include "ns/contiki.h"
#include "ns/sys/rtimer.h"
#include <stdbool.h>

// Alarm implementation for rtimer schedule
static bool is_ms_running = false;
static uint32_t ms_alarm = 0;

static uint32_t rtimer_alarm_milli_get_now(void)
{
    return (uint32_t)(clock_time() / RTIMER_ARCH_SECOND);
}

static void rtimer_alarm_milli_start_at(uint32_t t0, uint32_t dt)
{
    ms_alarm = t0 + dt;
    is_ms_running = true;
}

static void rtimer_alarm_stop(void)
{
    is_ms_running = false;
}

void rtimer_alarm_process(void)
{
    int32_t remaining;
    if (is_ms_running) {
        remaining = (int32_t)(ms_alarm - rtimer_alarm_milli_get_now());
        if (remaining <= 0) {
            rtimer_alarm_stop();
            rtimer_run_next();
        }
    }
}

void rtimer_arch_init(void)
{
    // init by clock_init();
}

void rtimer_arch_schedule(rtimer_clock_t t)
{
    rtimer_alarm_milli_start_at(clock_time(), (uint32_t)t);
}

rtimer_clock_t rtimer_arch_now(void)
{
    return (rtimer_clock_t)rtimer_alarm_milli_get_now();
}
