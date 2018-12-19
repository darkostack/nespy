#ifndef NS_CORE_MAC_MAC_H_
#define NS_CORE_MAC_MAC_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/dataset.h"
#include "ns/include/platform/radio.h"
#include "ns/include/platform/time.h"

enum {
    MAC_MIN_BE              = 3,  // macMinBe (IEEE 802.15.4-2006)
    MAC_MAX_BE              = 5,  // macMaxBe (IEEE 802.15.4-2006)
    MAC_UNIT_BACKOFF_PERIOD = 20, // number of symbols (IEEE 802.15.4-2006)

    MAC_MIN_BACKOFF = 1, // minimum backoff (milliseconds)

    MAC_ACK_TIMEOUT       = 16,  // timeout for waiting on an ACK (milliseconds)
    MAC_DATA_POLL_TIMEOUT = 100, // timeout for receiving data frame (milliseconds)
    MAC_SLEEP_DELAY       = 300, // max sleep delay when frame is pending (milliseconds)
    MAC_NONCE_SIZE        = 13,  // size of IEEE 802.15.4 Nonce (bytes)

    MAC_SCAN_CHANNEL_ALL      = NS_CHANNEL_ALL, // all channels
    MAC_SCAN_DURATION_DEFAULT = 300,            // default interval between channels (milliseconds)
};

#endif // NS_CORE_MAC_MAC_H_
