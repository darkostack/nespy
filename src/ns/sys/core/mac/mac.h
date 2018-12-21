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
#include "ns/sys/core/thread/topology.h"

typedef struct _mac mac_t;

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
    bool beacons_enabled : 1;
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

    mac_ext_addr_t ext_addr;
    mac_short_addr_t short_addr;
    mac_panid_t panid;
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

    success_rate_tracker_t cca_success_rate_tracker;
    uint16_t cca_sample_count;
    bool enabled;
};

void
mac_ctor(void *instance, mac_t *mac);

ns_error_t
mac_active_scan(mac_t *mac,
                uint32_t scan_channels,
                uint16_t scan_duration,
                active_scan_handler_func_t handler,
                void *context);

ns_error_t
mac_convert_beacon_to_active_scan_result(mac_t *mac,
                                         mac_frame_t *beacon_frame,
                                         ns_active_scan_result_t *result);

ns_error_t
mac_energy_scan(mac_t *mac,
                uint32_t scan_channels,
                uint16_t scan_duration,
                energy_scan_handler_func_t handler,
                void *context);

void
mac_energy_scan_done(mac_t *mac, int8_t rssi);

bool
mac_is_beacon_enabled(mac_t *mac);

void
mac_set_beacon_enabled(mac_t *mac, bool enabled);

bool
mac_get_rx_on_when_idle(mac_t *mac);

void
mac_set_rx_on_when_idle(mac_t *mac, bool rx_on_when_idle);

ns_error_t
mac_send_frame_request(mac_t *mac);

ns_error_t
mac_send_out_of_band_frame_request(mac_t *mac, ns_radio_frame_t *oob_frame);

const mac_ext_addr_t *
mac_get_ext_addr(mac_t *mac);

void
mac_set_ext_addr(mac_t *mac, const mac_ext_addr_t *ext_addr);

mac_short_addr_t
mac_get_short_addr(mac_t *mac);

ns_error_t
mac_set_short_addr(mac_t *mac, mac_short_addr_t short_addr);

uint8_t
mac_get_pan_channel(mac_t *mac);

ns_error_t
mac_set_pan_channel(mac_t *mac, uint8_t channel);

uint8_t
mac_get_radio_channel(mac_t *mac);

ns_error_t
mac_set_radio_channel(mac_t *mac, uint16_t acquisition_id, uint8_t channel);

ns_error_t
mac_acquired_radio_channel(mac_t *mac, uint16_t *acquisition_id);

ns_error_t
mac_release_radio_channel(mac_t *mac);

const mac_channel_mask_t
mac_get_supported_channel_mask(mac_t *mac);

void
mac_set_supported_channel_mask(mac_t *mac, const mac_channel_mask_t mask);

const char *
mac_get_network_name(mac_t *mac);

ns_error_t
mac_set_network_name(mac_t *mac, const char *network_name);

ns_error_t
mac_set_network_name_buf(mac_t *mac, const char *buffer, uint8_t length);

uint16_t
mac_get_panid(mac_t *mac);

ns_error_t
mac_set_panid(mac_t *mac, uint16_t panid);

const ns_extended_panid_t *
mac_get_extended_panid(mac_t *mac);

ns_error_t
mac_set_extended_panid(mac_t *mac, const ns_extended_panid_t *extended_panid);

#if NS_ENABLE_MAC_FILTER
mac_filter_t
mac_get_filter(mac_t *mac);
#endif

void
mac_handle_received_frame(mac_t *mac, mac_frame_t *frame, ns_error_t error);

void
mac_handle_transmit_started(mac_t *mac, ns_radio_frame_t *frame);

void
mac_handle_transmit_done(mac_t *mac, ns_radio_frame_t *frame, ns_radio_frame_t *ack_frame, ns_error_t error);

bool
mac_is_active_scan_in_progress(mac_t *mac);

bool
mac_is_energy_scan_in_progress(mac_t *mac);

bool
mac_is_in_transmit_state(mac_t *mac);

void
mac_set_pcap_callback(mac_t *mac, ns_link_pcap_callback_func_t pcap_callback, void *context);

bool
mac_is_promiscuous(mac_t *mac);

void
mac_set_promiscuous(mac_t *mac, bool promiscuous);

void
mac_reset_counters(mac_t *mac);

ns_mac_counters_t *
mac_get_counters(mac_t *mac);

int8_t
mac_get_noise_floor(mac_t *mac);

bool
mac_radio_supports_csma_backoffs(mac_t *mac);

bool
mac_radio_supports_retries(mac_t *mac);

uint16_t
mac_get_cca_failure_rate(mac_t *mac);

ns_error_t
mac_set_enabled(mac_t *mac);

bool
mac_is_enabled(mac_t *mac);

void
mac_process_transmit_aes_ccm(mac_t *mac, mac_frame_t *frame, const mac_ext_addr_t *ext_addr);

#endif // NS_CORE_MAC_MAC_H_
