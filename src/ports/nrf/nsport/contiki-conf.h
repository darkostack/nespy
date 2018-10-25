#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>
#include <inttypes.h>

// include project specific conf
#ifdef PROJECT_CONF_PATH
#include PROJECT_CONF_PATH
#endif

#define CLOCK_CONF_SECOND 1 // TODO: based on RTC seconds

typedef unsigned long clock_time_t;
typedef unsigned int uip_stats_t;

#endif // CONTIKI_CONF_H_
