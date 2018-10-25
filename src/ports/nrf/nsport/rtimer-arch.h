#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#include "contiki.h"
#include <nrfx.h>

#define RTIMER_ARCH_SECOND (SystemCoreClock) // we use system ticks for RTIMER

rtimer_clock_t rtimer_arch_now(void);

#endif // RTIMER_ARCH_H_
