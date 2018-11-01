#include "ns/contiki.h"
#include "ns/sys/etimer.h"
#include "ns/sys/stimer.h"
#include "ns/sys/timer.h"
#include "ns/sys/rtimer.h"
#include "ns/modules/nstd.h"
#include <stdio.h>

PROCESS(timer_test_process, "timer test process");
AUTOSTART_PROCESSES(&timer_test_process);

static int counter_etimer;
static int counter_timer;
static int counter_stimer;
static int counter_rtimer;
static struct timer timer_timer;
static struct stimer timer_stimer;
static struct ctimer timer_ctimer;
static struct rtimer timer_rtimer;

void ctimer_callback(void *ptr)
{
    ctimer_reset(&timer_ctimer);
    ns_log("ctimer callback called");
}

void rtimer_callback(struct rtimer *timer, void *ptr)
{
    ns_log("rtimer callback called: %d", counter_rtimer++);
    rtimer_set(&timer_rtimer, RTIMER_NOW() + RTIMER_SECOND / 2, 0,
               rtimer_callback, NULL);
}

PROCESS_THREAD(timer_test_process, ev, data)
{
    static struct etimer timer_etimer;

    PROCESS_BEGIN();

    ns_log("timer test process start");

    ctimer_set(&timer_ctimer, CLOCK_SECOND, ctimer_callback, NULL);
    rtimer_set(&timer_rtimer, RTIMER_NOW() + RTIMER_SECOND / 2, 0,
               rtimer_callback, NULL);

    while (1) {
        timer_set(&timer_timer, 3 * CLOCK_SECOND);
        stimer_set(&timer_stimer, 3);
        etimer_set(&timer_etimer, 3 * CLOCK_SECOND);

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

        counter_etimer++;
        if (timer_expired(&timer_timer)) {
            counter_timer++;
        }

        if (stimer_expired(&timer_stimer)) {
            counter_stimer++;
        }

        ns_log("timer process: -------- %s", counter_timer == counter_etimer &&
               counter_timer == counter_stimer ? "SUCCESS" : "FAIL");
    }

    PROCESS_END();
}
