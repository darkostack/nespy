#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>
#include <inttypes.h>

// include project specific conf
#ifdef PROJECT_CONF_PATH
#include PROJECT_CONF_PATH
#endif

#define MS_PER_S 1000
#define US_PER_MS 1000
#define US_PER_S 1000000

#define CLOCK_CONF_SECOND 1000000
#define RTIMER_ARCH_SECOND 1000

typedef unsigned long clock_time_t;
typedef unsigned int uip_stats_t;

#endif // CONTIKI_CONF_H_
