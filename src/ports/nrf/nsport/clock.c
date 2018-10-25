#include "contiki.h"
#include "nrfx_systick.h"
#include "nrf_systick.h"

volatile unsigned long clock_seconds_counter = 0;

void clock_init(void)
{
    nrfx_systick_init();
}

clock_time_t clock_time(void)
{
    // get the current clock time, measured in system ticks
    return nrf_systick_val_get();
}

void clock_set_seconds(unsigned long sec)
{
    // set the current value of the platform seconds
    clock_seconds_counter = sec;
}

unsigned long clock_seconds(void)
{
    // get the current value of the platform seconds.
    // this could be the number of seconds since startup or since standard epoch
    return clock_seconds_counter;
}

void clock_wait(clock_time_t i)
{
    // delay given number of ticks
    nrfx_systick_delay_ticks((uint32_t)i);
}

void clock_delay_usec(uint16_t dt)
{
    // delay given number of microseconds
    nrfx_systick_delay_us(dt);
}

void clock_delay(unsigned int i)
{
    // delay given number of milliseconds
    nrfx_systick_delay_ms((uint32_t)i);
}
