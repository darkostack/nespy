#include "contiki.h"

void clock_init(void)
{

}

clock_time_t clock_time(void)
{
    return 0;
}

void clock_set_seconds(unsigned long sec)
{
    (void) sec;
}

unsigned long clock_seconds(void)
{
    return 0;
}

void clock_wait(clock_time_t i)
{
    (void) i;
}

void clock_delay_usec(uint16_t dt)
{
    (void) dt;
}

void clock_delay(unsigned int i)
{

}

static void update_ticks(void)
{

}

void clock_adjust(void)
{

}

void clock_isr(void)
{
    update_ticks();
}
