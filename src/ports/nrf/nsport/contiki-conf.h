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
#define RTC_CONF_FREQUENCY      (32768UL)
#define RTC_CONF_IRQ_PRIORITY   7 // 3 bit priority (7: lowest, 0: highest)

#define CLOCK_CONF_SECOND       RTC_CONF_FREQUENCY

typedef unsigned long clock_time_t;
typedef unsigned int uip_stats_t;

#endif // CONTIKI_CONF_H_
