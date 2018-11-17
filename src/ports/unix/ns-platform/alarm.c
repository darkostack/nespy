#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ns/include/platform/alarm.h"
#include "ns-platform/platform-unix.h"

#define MS_PER_S 1000
#define US_PER_MS 1000
#define US_PER_S 1000000

#define DEFAULT_TIMEOUT 10 // seconds

static bool is_ms_running = false;
static uint32_t ms_alarm = 0;

static struct timeval start;

void plat_alarm_init(void)
{
    gettimeofday(&start, NULL);
}

uint64_t plat_alarm_get_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timersub(&tv, &start, &tv);
    return (uint64_t)tv.tv_sec * US_PER_S + (uint64_t)tv.tv_usec;
}

uint32_t ns_plat_alarm_get_now(void)
{
    return (uint32_t)(plat_alarm_get_now() / US_PER_MS);
}

void ns_plat_alarm_start_at(uint32_t t0, uint32_t dt)
{
    ms_alarm = t0 + dt;
    is_ms_running = true;
}

void ns_plat_alarm_stop(void)
{
    is_ms_running = false;
}

void plat_alarm_update_timeout(struct timeval *timeout)
{
    int32_t ms_remaining = DEFAULT_TIMEOUT *MS_PER_S;

    if (timeout == NULL) {
        return;
    }

    if (is_ms_running) {
        ms_remaining = (int32_t)(ms_alarm - ns_plat_alarm_get_now());
    }

    if (ms_remaining <= 0) {
        timeout->tv_sec = 0;
        timeout->tv_usec = 0;
    } else {
        int64_t remaining = ((int64_t)ms_remaining) * US_PER_MS;
        if (remaining == 0) {
            remaining = 1;
        }
        timeout->tv_sec = (time_t)remaining / US_PER_S;
        timeout->tv_usec = remaining % US_PER_S;
    }
}

void plat_alarm_process(ns_instance_t instance)
{
    int32_t remaining;

    if (is_ms_running) {
        remaining = (int32_t)(ms_alarm - ns_plat_alarm_get_now());
        if (remaining <= 0) {
            is_ms_running = false;
            ns_plat_alarm_fired(instance);
        }
    }
}
