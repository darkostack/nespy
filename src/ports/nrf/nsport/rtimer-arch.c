#include "contiki.h"
#include "ns/sys/rtimer.h"

void rtimer_arch_init(void)
{
    return;
}

void rtimer_arch_schedule(rtimer_clock_t t)
{
    (void) t;
}

rtimer_clock_t rtimer_arch_next_trigger(void)
{
    return 0;
}

rtimer_clock_t rtimer_arch_now(void)
{
    return 0;
}

void rtimer_isr()
{
    return;
}
