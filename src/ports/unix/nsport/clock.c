#include "ns/contiki.h"
#include "port_unix.h"

#define MS_PER_S 1000
#define US_PER_MS 1000
#define US_PER_S 1000000

static struct timeval start;

void clock_init(void)
{
    gettimeofday(&start, NULL);
}

clock_time_t clock_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timersub(&tv, &start, &tv);
    return (clock_time_t)tv.tv_sec * US_PER_S + (clock_time_t)tv.tv_usec;
}

unsigned long clock_seconds(void)
{
    return (unsigned long)(clock_time() / US_PER_S);
}

void clock_wait(clock_time_t i)
{
    clock_time_t t0;
    t0 = clock_time();
    while (clock_time() - t0 < (clock_time_t)i) {
        // do nothing
    }
}

void clock_delay_usec(uint16_t dt)
{
    clock_wait(dt);
}

void clock_delay(unsigned int i)
{
    clock_delay_usec(i);
}

void etimer_pending_process(void)
{
    if ((clock_time() % US_PER_MS) == 0) {
        if (etimer_pending()) {
            etimer_request_poll();
        }
    }
}
