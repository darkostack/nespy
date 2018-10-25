#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#include "contiki.h"

#define RTIMER_ARCH_SECOND CLOCK_CONF_SECOND

rtimer_clock_t rtimer_arch_next_trigger(void);

rtimer_clock_t rtimer_arch_now(void);

#endif // RTIMER_ARCH_H_
