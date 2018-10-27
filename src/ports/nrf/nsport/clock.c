#include "ns/contiki.h"
#include "nrfx_systick.h"
#include "nrfx_rtc.h"

static volatile uint32_t rtc_ticks;
static volatile uint32_t rtc_epoch;

static const nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(0);
static const nrfx_rtc_config_t rtc_config = {
    .prescaler = RTC_FREQ_TO_PRESCALER(RTC_CONF_FREQUENCY),
    .reliable = 0,
    .tick_latency = 0, // ignored when reliable == 0
    .interrupt_priority = RTC_CONF_IRQ_PRIORITY,
};

static void clock_update(void);

static void rtc_handler(nrfx_rtc_int_type_t int_type)
{
    if (int_type == NRFX_RTC_INT_TICK) {
        clock_update();
    }
}

void clock_init(void)
{
    // systick use for delay functions and won't generate the interrupt
    nrfx_systick_init();

    // make sure it's uninitialized.
    rtc_ticks = 0;
    rtc_epoch = 0;
    nrfx_rtc_uninit(&rtc);
    nrfx_rtc_counter_clear(&rtc);

    // initialize rtc
    nrfx_rtc_init(&rtc, &rtc_config, rtc_handler);

    // enable tick event & interrrupt
    nrfx_rtc_tick_enable(&rtc, true);

    // start RTC
    nrfx_rtc_enable(&rtc);
}

clock_time_t clock_time(void)
{
    return (clock_time_t)(rtc_ticks & 0xFFFFFFFF);
}

unsigned long clock_seconds(void)
{
    return (unsigned long)rtc_epoch;
}

void clock_wait(clock_time_t i)
{
    clock_time_t start;
    start = clock_time();
    while (clock_time() - start < (clock_time_t)i) {
        __WFE();
    }
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

static void clock_update(void)
{
    rtc_ticks++;
    if ((rtc_ticks % CLOCK_CONF_SECOND) == 0) {
        rtc_epoch++;
    }
    if (etimer_pending()) {
        etimer_request_poll();
    }
}
