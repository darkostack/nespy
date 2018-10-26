#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>
#include <inttypes.h>

// include project specific conf
#ifdef PROJECT_CONF_PATH
#include PROJECT_CONF_PATH
#endif

// rtc configuration used by clock.c
#define RTC_CONF_FREQUENCY (32768UL)
#define RTC_CONF_IRQ_PRIORITY 7 // 3bit priority (7: lowest, 0: highest)

#define CLOCK_CONF_SECOND       RTC_CONF_FREQUENCY

// timer configuration used by rtimer-arch.c
#define RTIMER_CONF_FREQ_31_25_KHz 9 // 9: NRFx timer 31.25KHz
#define RTIMER_CONF_MODE 0 // 0: NRFx timer mode
#define RTIMER_CONF_CNT_BIT_WIDTH 3 // 3: NRFx timer bit width 32bit
#define RTIMER_CONF_IRQ_PRIO 6 

#define RTIMER_ARCH_SECOND      (31250UL) // timer 2 at 31.25KHz

typedef unsigned long clock_time_t;
typedef unsigned int uip_stats_t;

#endif // CONTIKI_CONF_H_
