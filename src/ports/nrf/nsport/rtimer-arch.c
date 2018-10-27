#include "ns/contiki.h"
#include "ns/sys/rtimer.h"
#include "nrfx_timer.h"

// Note: `rtimer` function callback is run inside ISR and it's priority is
// higher than `clock` (RTC) priority that run the etimer (processes).
// So don't spend too much time at `rtimer` function callback.

static const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(2);
static const nrfx_timer_config_t timer_config = {
    .frequency = (nrf_timer_frequency_t)RTIMER_CONF_FREQ_31_25_KHz,
    .mode = (nrf_timer_mode_t)RTIMER_CONF_MODE,
    .bit_width = (nrf_timer_bit_width_t)RTIMER_CONF_CNT_BIT_WIDTH,
    .interrupt_priority = RTIMER_CONF_IRQ_PRIO,
    .p_context = NULL
};

static void timer_handler(nrf_timer_event_t event_type, void *p_context)
{
    if (event_type == NRF_TIMER_EVENT_COMPARE2) {
        rtimer_run_next();
    }
}

void rtimer_arch_init(void)
{
    nrfx_timer_init(&timer, &timer_config, timer_handler);
    nrfx_timer_enable(&timer);
}

void rtimer_arch_schedule(rtimer_clock_t t)
{
    nrfx_timer_compare(&timer, NRF_TIMER_CC_CHANNEL2, (uint32_t)t, true);
}

rtimer_clock_t rtimer_arch_now(void)
{
    return (rtimer_clock_t)nrfx_timer_capture(&timer, NRF_TIMER_CC_CHANNEL2);
}

