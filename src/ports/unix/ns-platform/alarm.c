#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ns/include/platform/alarm-milli.h"
#include "ns/include/platform/alarm-micro.h"
#include "platform-unix.h"

#define MS_PER_S 1000
#define US_PER_MS 1000
#define US_PER_S 1000000

#define DEFAULT_TIMEOUT 10 // seconds

static bool     s_is_ms_running = false;
static uint32_t s_ms_alarm = 0;

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
static bool     s_is_us_running = false;
static uint32_t s_us_alarm = 0;
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

static struct timeval s_start;

void
plat_alarm_init(void)
{
    gettimeofday(&s_start, NULL);
}

uint64_t
plat_alarm_get_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timersub(&tv, &s_start, &tv);
    return (uint64_t)tv.tv_sec * US_PER_S + (uint64_t)tv.tv_usec;
}

uint32_t
ns_plat_alarm_milli_get_now(void)
{
    return (uint32_t)(plat_alarm_get_now() / US_PER_MS);
}

void
ns_plat_alarm_milli_start_at(ns_instance_t instance, uint32_t t0, uint32_t dt)
{
    (void)instance;
    s_ms_alarm = t0 + dt;
    s_is_ms_running = true;
}

void
ns_plat_alarm_milli_stop(ns_instance_t instance)
{
    (void)instance;
    s_is_ms_running = false;
}

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
uint32_t
ns_plat_alarm_micro_get_now(void)
{
    return (uint32_t)plat_alarm_get_now();
}

void
ns_plat_alarm_micro_start_at(ns_instance_t instance, uint32_t t0, uint32_t dt)
{
    (void)instance;
    s_us_alarm = t0 + dt;
    s_is_us_running = true;
}

void
ns_plat_alarm_micro_stop(ns_instance_t instance)
{
    (void)instance;
    s_is_us_running = false;
}
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

void
plat_alarm_update_timeout(struct timeval *timeout)
{
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    int32_t us_remaining = DEFAULT_TIMEOUT *US_PER_S;
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    int32_t ms_remaining = DEFAULT_TIMEOUT *MS_PER_S;

    if (timeout == NULL) {
        return;
    }

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    if (s_is_us_running) {
        us_remaining = (int32_t)(s_us_alarm - ns_plat_alarm_micro_get_now());
    }
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

    if (s_is_ms_running) {
        ms_remaining = (int32_t)(s_ms_alarm - ns_plat_alarm_milli_get_now());
    }

    bool is_no_remaining;

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    is_no_remaining = (us_remaining <= 0 || ms_remaining <= 0) ? true : false;
#else
    is_no_remaining = (ms_remaining <= 0) ? true : false;
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

    if (is_no_remaining) {
        timeout->tv_sec = 0;
        timeout->tv_usec = 0;
    } else {
        int64_t remaining = ((int64_t)ms_remaining) * US_PER_MS;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
        if (us_remaining < remaining) {
            remaining = us_remaining;
        }
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
        if (remaining == 0) {
            remaining = 1;
        }
        timeout->tv_sec = (time_t)remaining / US_PER_S;
        timeout->tv_usec = remaining % US_PER_S;
    }
}

void
plat_alarm_process(ns_instance_t instance)
{
    int32_t remaining;

    if (s_is_ms_running) {
        remaining = (int32_t)(s_ms_alarm - ns_plat_alarm_milli_get_now());
        if (remaining <= 0) {
            s_is_ms_running = false;
            ns_plat_alarm_milli_fired(instance);
        }
    }
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    if (s_is_us_running) {
        remaining = (int32_t)(s_us_alarm - ns_plat_alarm_micro_get_now());
        if (remaining <= 0) {
            s_is_us_running = false;
            ns_plat_alarm_micro_fired(instance);
        }
    }
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
}

#if NS_CONFIG_ENABLE_TIME_SYNC
uint64_t ns_plat_time_get(void)
{
    return plat_alarm_get_now();
}

uint16_t
ns_plat_time_get_xtal_accuracy(void)
{
    return 0;
}
#endif // NS_CONFIG_ENABLE_TIME_SYNC
