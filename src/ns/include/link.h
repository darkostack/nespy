#ifndef NS_LINK_H_
#define NS_LINK_H_

#include "ns/include/dataset.h"
#include "ns/include/platform/radio.h"

typedef struct _ns_thread_link_info {
    uint16_t panid;     // source PAN ID
    uint8_t channel;    // 802.15.4 channel
    int8_t rss;         // received signal strength in dBm
    uint8_t lqi;        // link quality indicator for a received message
    bool link_security; // indicates whether or not link security is enabled

    // applicable/required only when time sync feature (`NS_CONFIG_ENABLE_TIME_SYNC`) is enabled
    uint8_t time_sync_seq;       // the time sync sequence
    int64_t network_time_offset; // the time offset to the Thread network time, in microseconds
} ns_thread_link_info_t;

#define NS_MAC_FILTER_FIXED_RSS_DISABLED 127
#define NS_MAC_FILTER_ITERATOR_INIT 0

typedef uint8_t ns_mac_filter_iterator_t;

typedef enum _ns_mac_filter_addr_mode {
    NS_MAC_FILTER_ADDRESS_MODE_DISABLED,
    NS_MAC_FILTER_ADDRESS_MODE_WHITELIST,
    NS_MAC_FILTER_ADDRESS_MODE_BLACKLIST,
} ns_mac_filter_addr_mode_t;

typedef struct _ns_mac_filter_entry {
    ns_ext_addr_t ext_addr;
    int8_t rss_in;
    bool filtered;
} ns_mac_filter_entry_t;

#endif // NS_LINK_H_
