#ifndef NS_CORE_MAC_LINK_RAW_H_
#define NS_CORE_MAC_LINK_RAW_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/link_raw.h"
#include "ns/sys/core/common/timer.h"
#include "ns/sys/core/common/tasklet.h"
#include "ns/sys/core/mac/mac_frame.h"

#if NS_RADIO || NS_ENABLE_RAW_LINK_API

#if NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT || NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF || NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
#define NS_MAC_LINK_RAW_TIMER_REQUIRED 1
#else
#define NS_MAC_LINK_RAW_TIMER_REQUIRED 0
#endif

typedef struct _mac_link_raw mac_link_raw_t;

typedef enum _mac_link_raw_timer_reason {
    MAC_LINK_RAW_TIMER_REASON_NONE,
    MAC_LINK_RAW_TIMER_REASON_ACK_TIMEOUT,
    MAC_LINK_RAW_TIMER_REASON_CSMA_BACKOFF_COMPLETE,
    MAC_LINK_RAW_TIMER_REASON_ENERGY_SCAN_COMPLETE,
} mac_link_raw_timer_reason_t;

#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
enum {
    MAC_LINK_RAW_INVALID_RSSI_VALUE = 127,
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    MAC_LINK_RAW_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL = 128,
#else
    MAC_LINK_RAW_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL = 1,
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
};
#endif // NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN

struct _mac_link_raw {
    void *instance;
    tasklet_t operation_task;
    bool pending_transmit_data;
#if NS_MAC_LINK_RAW_TIMER_REQUIRED
    timer_milli_t timer;
    mac_link_raw_timer_reason_t timer_reason;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_micro_t timer_micro;
#else
    timer_milli_t energy_scan_timer;
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
#endif // NS_MAC_LINK_RAW_TIMER_REQUIRED
#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
    uint8_t csma_backoffs;
#endif // NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
#if NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
    uint8_t transmit_retries;
#endif // NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
    int8_t energy_scan_rssi;
#endif // NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
    ns_ext_addr_t ext_addr;
    uint16_t panid;
    uint16_t short_addr;
    bool enabled;
    uint8_t receive_channel;
    ns_link_raw_receive_done_func_t receive_done_callback;
    ns_link_raw_transmit_done_func_t transmit_done_callback;
    ns_link_raw_energy_scan_done_func_t energy_scan_done_callback;
    mac_frame_t *transmit_frame;
    ns_radio_caps_t radio_caps;
};

void
mac_link_raw_ctor(void *instance, mac_link_raw_t *link_raw);

bool
mac_link_raw_is_enabled(mac_link_raw_t *link_raw);

ns_error_t
mac_link_raw_set_enabled(mac_link_raw_t *link_raw, bool enabled);

ns_radio_caps_t
mac_link_raw_get_caps(mac_link_raw_t *link_raw);

ns_error_t
mac_link_raw_receive(mac_link_raw_t *link_raw, ns_link_raw_receive_done_func_t callback);

void
mac_link_raw_invoke_receive_done(mac_link_raw_t *link_raw, ns_radio_frame_t *frame, ns_error_t error);

ns_error_t
mac_link_raw_transmit(mac_link_raw_t *link_raw, ns_link_raw_transmit_done_func_t callback);

void
mac_link_raw_invoke_transmit_done(mac_link_raw_t *link_raw,
                                  ns_radio_frame_t *frame,
                                  ns_radio_frame_t *ack_frame,
                                  ns_error_t error);

ns_error_t
mac_link_raw_energy_scan(mac_link_raw_t *link_raw,
                         uint8_t scan_channel,
                         uint16_t scan_duration,
                         ns_link_raw_energy_scan_done_func_t callback);

void
mac_link_raw_invoke_energy_scan_done(mac_link_raw_t *link_raw, int8_t energy_scan_max_rssi);

void
mac_link_raw_transmit_started(mac_link_raw_t *link_raw, ns_radio_frame_t *frame);

uint16_t
mac_link_raw_get_short_addr(mac_link_raw_t *link_raw);

ns_error_t
mac_link_raw_set_short_addr(mac_link_raw_t *link_raw, uint16_t short_addr);

uint16_t
mac_link_raw_get_panid(mac_link_raw_t *link_raw);

ns_error_t
mac_link_raw_set_panid(mac_link_raw_t *link_raw, uint16_t panid);

uint8_t
mac_link_raw_get_channel(mac_link_raw_t *link_raw);

ns_error_t
mac_link_raw_set_channel(mac_link_raw_t *link_raw, uint8_t channel);

ns_ext_addr_t *
mac_link_raw_get_ext_addr(mac_link_raw_t *link_raw);

ns_error_t
mac_link_raw_set_ext_addr(mac_link_raw_t *link_raw, ns_ext_addr_t ext_addr);

ns_radio_frame_t *
mac_link_raw_get_transmit_frame(mac_link_raw_t *link_raw);

#endif // #if NS_RADIO || NS_ENABLE_RAW_LINK_API

#endif // NS_CORE_MAC_LINK_RAW_H_
