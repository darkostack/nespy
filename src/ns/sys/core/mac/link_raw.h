#ifndef NS_CORE_MAC_LINK_RAW_H_
#define NS_CORE_MAC_LINK_RAW_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/link_raw.h"
#include "ns/sys/core/common/timer.h"
#include "ns/sys/core/common/tasklet.h"
#include "ns/sys/core/mac/mac_frame.h"

#if NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT || NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF || NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
#define NS_LINK_RAW_TIMER_REQUIRED 1
#else
#define NS_LINK_RAW_TIMER_REQUIRED 0
#endif

typedef enum _link_raw_timer_reason {
    LINK_RAW_TIMER_REASON_NONE,
    LINK_RAW_TIMER_REASON_ACK_TIMEOUT,
    LINK_RAW_TIMER_REASON_CSMA_BACKOFF_COMPLETE,
    LINK_RAW_TIMER_REASON_ENERGY_SCAN_COMPLETE,
} link_raw_timer_reason_t;

struct _link_raw {
    void *instance;
    tasklet_t operation_task;
    bool pending_transmit_data;
#if NS_LINK_RAW_TIMER_REQUIRED
    timer_milli_t timer;
    link_raw_timer_reason_t timer_reason;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_micro_t timer_micro;
#else
    timer_milli_t energy_scan_timer;
#endif
#endif // NS_LINK_RAW_TIMER_REQUIRED
};

#endif // NS_CORE_MAC_LINK_RAW_H_
