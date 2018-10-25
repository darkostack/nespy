#include "contiki.h"
#include "nrfx_systick.h"
#include "nrf_systick.h"

volatile unsigned long clock_seconds_counter = 0;

void clock_init(void)
{
    nrfx_systick_init();
}

// get the current clock time, measured in system ticks
clock_time_t clock_time(void)
{
    return nrf_systick_val_get();
}

// set the current value of the platform seconds
void clock_set_seconds(unsigned long sec)
{
    clock_seconds_counter = sec;
}

// get the current value of the platform seconds.
// this could be the number of seconds since startup or since standard epoch
unsigned long clock_seconds(void)
{
    return clock_seconds_counter;
}

// delay given number of ticks
void clock_wait(clock_time_t i)
{
    nrfx_systick_delay_ticks((uint32_t)i);
}

// delay given number of microseconds
void clock_delay_usec(uint16_t dt)
{
    nrfx_systick_delay_us(dt);
}

// delay given number of milliseconds
void clock_delay(unsigned int i)
{
    nrfx_systick_delay_ms((uint32_t)i);
}
