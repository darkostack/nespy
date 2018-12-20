#ifndef NS_CORE_MAC_MAC_H_
#define NS_CORE_MAC_MAC_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/dataset.h"
#include "ns/include/platform/radio.h"
#include "ns/include/platform/time.h"
#include "ns/sys/core/common/tasklet.h"
#include "ns/sys/core/common/timer.h"
#include "ns/sys/core/mac/channel_mask.h"
#include "ns/sys/core/mac/mac_filter.h"
#include "ns/sys/core/mac/mac_frame.h"
#include "ns/sys/core/thread/link_quality.h"

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

    MAC_MAX_CSMA_BACKOFFS_DIRECT   = NS_CONFIG_MAC_MAX_CSMA_BACKOFFS_DIRECT,
    MAC_MAX_CSMA_BACKOFFS_INDIRECT = NS_CONFIG_MAC_MAX_CSMA_BACKOFFS_INDIRECT,
    
    MAC_MAX_FRAME_RETRIES_DIRECT   = NS_CONFIG_MAC_MAX_FRAME_RETRIES_DIRECT,
    MAC_MAX_FRAME_RETRIES_INDIRECT = NS_CONFIG_MAC_MAX_FRAME_RETRIES_INDIRECT,

    MAC_TX_NUM_BCAST = NS_CONFIG_TX_NUM_BCAST
};

enum {
    MAC_INVALID_RSSI_VALUE     = 127,
    MAC_MAX_CCA_SAMPLE_COUNT   = NS_CONFIG_CCA_FAILURE_RATE_AVERAGING_WINDOW,
    MAC_MAX_ACQUISITION_ID     = 0xffff,

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    MAC_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL = 128,
#else
    MAC_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL = 1,
#endif
};

typedef enum _mac_operation {
    MAC_OPERATION_IDLE = 0,
    MAC_OPERATION_ACTIVE_SCAN,
    MAC_OPERATION_ENERGY_SCAN,
    MAC_OPERATION_TRANSMIT_BEACON,
    MAC_OPERATION_TRANSMIT_DATA,
    MAC_OPERATION_WAITING_FOR_DATA,
    MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME,
} mac_operation_t;

typedef void (*active_scan_handler_func_t)(void *context, mac_frame_t *beacon_frame);
typedef void (*energy_scan_handler_func_t)(void *context, ns_energy_scan_result_t *result);

struct _mac {
    void *instance;

    mac_operation_t operation;

    bool pending_active_scan : 1;
    bool pending_energy_scan : 1;
    bool pending_transmit_beacon : 1;
    bool pending_transmit_data : 1;
    bool pending_transmit_oob_frame : 1;
    bool pending_waiting_for_data : 1;
    bool rx_on_when_idle : 1;
    bool beacon_enabled : 1;
    bool transmit_aborted : 1;
#if NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS
    bool delay_sleep : 1;
#endif

    tasklet_t operation_task;

    timer_milli_t mac_timer;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_micro_t backoff_timer;
#else
    timer_milli_t backoff_timer;
#endif
    timer_milli_t receive_timer;

    ext_addr_t ext_addr;
    short_addr_t short_addr;
    panid_t panid;
    uint8_t pan_channel;
    uint8_t radio_channel;
    uint16_t radio_channel_acquisition_id;
    mac_channel_mask_t supported_channel_mask;

    ns_network_name_t network_name;
    ns_extended_panid_t extended_panid;

    uint8_t beacon_sequence;
    uint8_t data_sequence;
    uint8_t csma_backoffs;
    uint8_t transmit_retries;
    uint8_t broadcast_transmit_count;

    mac_channel_mask_t scan_channel_mask;
    uint16_t scan_duration;
    uint8_t scan_channel;
    int8_t energy_scan_current_max_rssi;
    void *scan_context;
    union {
        active_scan_handler_func_t active_scan_handler;
        energy_scan_handler_func_t energy_scan_handler;
    };

    ns_link_pcap_callback_func_t pcap_callback;
    void *pcap_callback_context;

#if NS_ENABLE_MAC_FILTER
    mac_filter_t filter;
#endif

    mac_frame_t *tx_frame;
    mac_frame_t *oob_frame;

    ns_mac_counters_t counters;
    uint32_t key_id_mode2_frame_counter;

    thread_link_quality_success_rate_tracker_t cca_success_rate_tracker;
    uint16_t cca_sample_count;
    bool enabled;
};

#endif // NS_CORE_MAC_MAC_H_
