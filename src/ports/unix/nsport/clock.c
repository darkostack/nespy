#include "ns/contiki.h"

void clock_init(void)
{
}

clock_time_t clock_time(void)
{
    return 0;
}

unsigned long clock_seconds(void)
{
    return 0;
}

void clock_wait(clock_time_t i)
{
    (void)i;
}

void clock_delay_usec(uint16_t dt)
{
    (void)dt;
}

void clock_delay(unsigned int i)
{
    clock_delay_usec(i);
}
