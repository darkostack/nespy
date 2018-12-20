#ifndef NS_LINK_H_
#define NS_LINK_H_

#include "ns/include/commissioner.h"
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

typedef struct _ns_mac_counters {
    uint32_t tx_total;                // The total number of transmissions.
    uint32_t tx_unicast;              // The total number of unicast transmissions.
    uint32_t tx_broadcast;            // The total number of broadcast transmissions.
    uint32_t tx_ack_requested;        // The number of transmissions with ack request.
    uint32_t tx_acked;                // The number of transmissions that were acked.
    uint32_t tx_no_ack_requested;     // The number of transmissions without ack request.
    uint32_t tx_data;                 // The number of transmitted data.
    uint32_t tx_data_poll;            // The number of transmitted data poll.
    uint32_t tx_beacon;               // The number of transmitted beacon.
    uint32_t tx_beacon_request;       // The number of transmitted beacon request.
    uint32_t tx_other;                // The number of transmitted other types of frames.
    uint32_t tx_retry;                // The number of retransmission times.
    uint32_t tx_err_cca;              // The number of CCA failure times.
    uint32_t tx_err_abort;            // The number of frame transmission failures due to abort error.
    uint32_t tx_err_busy_channel;     // The number of frames that were dropped due to a busy channel.
    uint32_t rx_total;                // The total number of received packets.
    uint32_t rx_unicast;              // The total number of unicast packets received.
    uint32_t rx_broadcast;            // The total number of broadcast packets received.
    uint32_t rx_data;                 // The number of received data.
    uint32_t rx_data_poll;            // The number of received data poll.
    uint32_t rx_beacon;               // The number of received beacon.
    uint32_t rx_beacon_request;       // The number of received beacon request.
    uint32_t rx_other;                // The number of received other types of frames.
    uint32_t rx_addr_filtered;        // The number of received packets filtered by address filter.
    uint32_t rx_dest_addr_filtered;   // The number of received packets filtered by destination check.
    uint32_t rx_duplicated;           // The number of received duplicated packets.
    uint32_t rx_err_no_frame;         // The number of received packets with no or malformed content.
    uint32_t rx_err_unknown_neighbor; // The number of received packets from unknown neighbor.
    uint32_t rx_err_invalid_src_addr; // The number of received packets whose source address is invalid.
    uint32_t rx_err_sec;              // The number of received packets with security error.
    uint32_t rx_err_fcs;              // The number of received packets with FCS error.
    uint32_t rx_err_other;            // The number of received packets with other error.
} ns_mac_counters_t;

// this structure represents a received IEEE 802.15.4 beacon
typedef struct _ns_active_scan_result {
    ns_ext_addr_t ext_addr;
    ns_network_name_t network_name;
    ns_extended_panid_t extended_panid;
    ns_steering_data_t steering_data;
    uint16_t panid;
    uint16_t joiner_udp_port;
    uint8_t channel;
    int8_t rssi;
    uint8_t lqi;
    unsigned int version: 4;
    bool is_native : 1;
    bool is_joinable : 1;
} ns_active_scan_result_t;

// this structure represent an energy scan result
typedef struct _ns_energy_scan_result {
    uint8_t channel;
    int8_t max_rssi;
} ns_energy_scan_result_t;

#endif // NS_LINK_H_
