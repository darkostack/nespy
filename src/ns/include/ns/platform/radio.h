#ifndef NS_PLATFORM_RADIO_H_
#define NS_PLATFORM_RADIO_H_

#include <stdint.h>
#include "ns/error.h"
#include "ns/instance.h"

enum {
    NS_RADIO_FRAME_MAX_SIZE = 127,
    NS_RADIO_CHANNEL_PAGE = 0,
    NS_RADIO_CHANNEL_MIN = 11,
    NS_RADIO_CHANNEL_MAX = 26,
    NS_RADIO_SUPPORTED_CHANNELS = 0xffff << NS_RADIO_CHANNEL_MIN,
    NS_RADIO_SYMBOLS_PER_OCTET = 2,
    NS_RADIO_BIT_RATE = 250000,

    NS_RADIO_BITS_PER_OCTET = 8,
    NS_RADIO_SYMBOL_TIME = ((NS_RADIO_BITS_PER_OCTET / NS_RADIO_SYMBOLS_PER_OCTET) * 1000000) / NS_RADIO_BIT_RATE,

    NS_RADIO_LQI_NONE = 0,
    NS_RADIO_RSSI_INVALID = 127,
};

typedef uint8_t ns_radio_caps_t;

enum {
    NS_RADIO_CAPS_NONE = 0,
    NS_RADIO_CAPS_ACK_TIMEOUT = 1 << 0,
    NS_RADIO_CAPS_ENERGY_SCAN = 1 << 1,
    NS_RADIO_CAPS_TRANSMIT_RETRIES = 1 << 2,
    NS_RADIO_CAPS_CSMA_BACKOFF = 1 << 3,
};

#define NS_PANID_BROADCAST 0xffff

typedef uint16_t ns_panid_t;

typedef uint16_t ns_short_addr_t;

#define NS_EXT_ADDRESS_SIZE 8

struct _ns_ext_addr {
    uint8_t m8[NS_EXT_ADDRESS_SIZE];
};

typedef struct _ns_ext_addr ns_ext_addr_t;

typedef struct _ns_radio_ie_info {
    uint8_t time_ie_offset;
    uint8_t time_sync_seq;
    uint64_t timestamp;
    int64_t network_time_offset;
} ns_radio_ie_info_t;

typedef struct _ns_radio_frame {
    uint8_t *psdu;
    uint8_t length;
    uint8_t channel;
    bool didtx : 1;
    ns_radio_ie_info_t *ie_info;
    union {
        struct {
            uint8_t max_csma_backoffs;
            uint8_t max_frame_retries;
            bool is_a_retx : 1;
            bool csma_ca_enabled : 1;
            const uint8_t *aes_key;
        } tx_info;
        struct {
            int8_t rssi;
            uint8_t lqi;
            uint16_t usec;
            uint32_t msec;
        } rx_info;
    } info;
} ns_radio_frame_t;

typedef enum _ns_radio_state {
    NS_RADIO_STATE_DISABLED = 0,
    NS_RADIO_STATE_SLEEP = 1,
    NS_RADIO_STATE_RECEIVE = 2,
    NS_RADIO_STATE_TRANSMIT = 3,
} ns_radio_state_t;

/**
 * The following are valid radio state transitions:
 *
 *                                    (Radio ON)
 *  +----------+  Enable()  +-------+  Receive() +---------+   Transmit()  +----------+
 *  |          |----------->|       |----------->|         |-------------->|          |
 *  | Disabled |            | Sleep |            | Receive |               | Transmit |
 *  |          |<-----------|       |<-----------|         |<--------------|          |
 *  +----------+  Disable() +-------+   Sleep()  +---------+   Receive()   +----------+
 *                                    (Radio OFF)                 or
 *                                                        signal TransmitDone
 */

ns_radio_caps_t
ns_plat_radio_get_caps(ns_instance_t instance);

const char *
ns_plat_radio_get_version_string(ns_instance_t instance);

int8_t
ns_plat_radio_get_receive_sensitivity(ns_instance_t instance);

void
ns_plat_radio_get_ieee_eui64(ns_instance_t instance, uint8_t *ieee_eui64);

void
ns_plat_radio_set_panid(ns_instance_t instance, ns_panid_t panid);

void
ns_plat_radio_set_extended_addr(ns_instance_t instance, const ns_ext_addr_t *ext_addr);

void
ns_plat_radio_set_short_addr(ns_instance_t instance, ns_short_addr_t short_addr);

ns_error_t
ns_plat_radio_get_transmit_power(ns_instance_t instance, int8_t *power);

ns_error_t
ns_plat_radio_set_transmit_power(ns_instance_t instance, int8_t power);

bool
ns_plat_radio_get_promiscuous(ns_instance_t instance);

void
ns_plat_radio_set_promiscuous(ns_instance_t instance, bool enable);

ns_radio_state_t
ns_plat_radio_get_state(ns_instance_t instance);

ns_error_t
ns_plat_radio_enable(ns_instance_t instance);

ns_error_t
ns_plat_radio_disable(ns_instance_t instance);

bool
ns_plat_radio_is_enabled(ns_instance_t instance);

ns_error_t
ns_plat_radio_sleep(ns_instance_t instance);

ns_error_t
ns_plat_radio_receive(ns_instance_t instance, uint8_t channel);

extern void
ns_plat_radio_receive_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_error_t error);

extern void
ns_plat_diag_radio_receive_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_error_t error);

ns_radio_frame_t *
ns_plat_radio_get_transmit_buffer(ns_instance_t instance);

ns_error_t
ns_plat_radio_transmit(ns_instance_t instance, ns_radio_frame_t *frame);

extern void
ns_plat_radio_tx_started(ns_instance_t instance, ns_radio_frame_t *frame);

extern void
ns_plat_radio_tx_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_radio_frame_t *ack_frame, ns_error_t error);

extern void
ns_plat_diag_radio_transmit_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_error_t error);

extern void
ns_plat_radio_frame_updated(ns_instance_t instance, ns_radio_frame_t *frame);

int8_t
ns_plat_radio_get_rssi(ns_instance_t instance);

ns_error_t
ns_plat_radio_energy_scan(ns_instance_t instance, uint8_t scan_channel, uint16_t scan_duration);

extern void
ns_plat_radio_energy_scan_done(ns_instance_t instance, int8_t energy_scan_max_rssi);

void
ns_plat_radio_enable_src_match(ns_instance_t instance, bool enable);

ns_error_t
ns_plat_radio_add_src_match_short_entry(ns_instance_t instance, ns_short_addr_t short_addr);

ns_error_t
ns_plat_radio_add_src_match_ext_entry(ns_instance_t instance, const ns_ext_addr_t *ext_addr);

ns_error_t
ns_plat_radio_clear_src_match_short_entry(ns_instance_t instance, ns_short_addr_t short_addr);

ns_error_t
ns_plat_radio_clear_src_match_ext_entry(ns_instance_t instance, const ns_ext_addr_t *ext_addr);

void
ns_plat_radio_clear_src_match_short_entries(ns_instance_t instance);

void
ns_plat_radio_clear_src_match_ext_entries(ns_instance_t instance);

#endif // NS_PLATFORM_RADIO_H_
