#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>
#include <inttypes.h>

// include project specific conf
#ifdef PROJECT_CONF_PATH
#include PROJECT_CONF_PATH
#endif

typedef unsigned long clock_time_t;

#define CLOCK_CONF_SECOND 1000

typedef unsigned int uip_stats_t;

#endif // CONTIKI_CONF_H_
