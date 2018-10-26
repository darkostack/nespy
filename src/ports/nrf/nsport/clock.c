#include "contiki.h"
#include "nrfx_systick.h"
#include "nrf_systick.h"

void clock_init(void)
{
    // systick use for delay functions and won't generate the interrupt
    nrfx_systick_init();
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
    // delay given number of microseconds
    nrfx_systick_delay_us(dt);
}

void clock_delay(unsigned int i)
{
    clock_delay_usec(i);
}
