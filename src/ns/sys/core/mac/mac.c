#include "ns/sys/core/mac/mac.h"
#include "ns/sys/core/common/code_utils.h"
#include "ns/sys/core/common/debug.h"
#include "ns/sys/core/common/encoding.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/random.h"
// TODO: #include "ns/sys/core/crypto/aes_ccm.h"
// TODO: #include "ns/sys/core/crypto/sha256.h"

static const uint8_t s_mode2_key[] = {0x78, 0x58, 0x16, 0x86, 0xfd, 0xb4, 0x58, 0x0f,
                                      0xb0, 0x92, 0x54, 0x6a, 0xec, 0xbd, 0x15, 0x66};

static const ns_error_t s_mode2_ext_addr = {
    {0x35, 0x06, 0xfe, 0xb8, 0x23, 0xd4, 0x87, 0x12},
};

static const ns_extended_panid_t s_extended_panid_init = {
    {0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xfe},
};

static const char s_network_name_init[] = "ns-thread";

// --- private functions declarations
static void
mac_process_transmit_security(mac_t *mac, mac_frame_t *frame, bool process_aes_ccm);

static void
mac_generate_nonce(mac_t *mac,
                   const mac_ext_addr_t *addr,
                   uint32_t frame_counter,
                   uint8_t security_level,
                   uint8_t *nonce);

static ns_error_t
mac_process_receive_security_frame(mac_t *mac,
                                   mac_frame_t *frame,
                                   const mac_addr_t *src_addr,
                                   neighbor_t *neighbor);

static void
mac_update_idle_mode(mac_t *mac);

static void
mac_start_operation(mac_t *mac, mac_operation_t operation);

static void
mac_finish_operation(mac_t *mac);

static void
mac_perform_next_operation(mac_t *mac);

static void
mac_send_beacon_request(mac_t *mac, mac_frame_t *frame);

static void
mac_send_beacon(mac_t *mac, mac_frame_t *frame);

static bool
mac_should_send_beacon(mac_t *mac);

static void
mac_start_backoff(mac_t *mac);

static void
mac_begin_transmit(mac_t *mac);

static ns_error_t
mac_handle_mac_command(mac_t *mac, mac_frame_t *frame);

static mac_frame_t *
mac_get_operation_frame(mac_t *mac);

static void
handle_mac_timer(void *timer);

static void
mac_handle_mac_timer(mac_t *mac);

static void
handle_backoff_timer(void *timer);

static void
mac_handle_backoff_timer(mac_t *mac);

static void
handle_receive_timer(void *timer);

static void
mac_handle_receive_timer(mac_t *mac);

static void
handle_operation_task(void *tasklet);

static void
mac_handle_operation_task(mac_t *mac);

static void
mac_start_csma_backoff(mac_t *mac);

static void
mac_scan(mac_t *mac,
         mac_operation_t scan_operation,
         uint32_t scan_channels,
         uint16_t scan_duration,
         void *context);

static ns_error_t
mac_update_scan_channel(mac_t *mac);

static void
mac_perform_active_scan(mac_t *mac);

static void
mac_perform_energy_scan(mac_t *mac);

static void
mac_report_energy_scan_result(mac_t *mac, int8_t rssi);

static void
mac_sample_rssi(mac_t *mac);

static ns_error_t
mac_radio_transmit(mac_t *mac, mac_frame_t *send_frame);

static ns_error_t
mac_radio_receive(mac_t *mac, uint8_t channel);

static ns_error_t
mac_radio_sleep(mac_t *mac);

static void
mac_log_frame_rx_failure(mac_t *mac, const mac_frame_t *frame, ns_error_t error);

static void
mac_log_frame_tx_failure(mac_t *mac, const mac_frame_t *frame, ns_error_t error);

static void
mac_log_beacon(mac_t *mac, const char *action_text, const mac_beacon_payload_t *beacon_payload);

#if NS_CONFIG_ENABLE_TIME_SYNC
static void
mac_process_time_ie(mac_t *mac, mac_frame_t *frame);

static uint8_t
mac_get_time_ie_offset(mac_t *mac, mac_frame_t *frame);
#endif

static const char *
mac_operation_to_string(mac_t *mac, mac_operation_t operation);

// --- mac functions
void
mac_ctor(void *instance, mac_t *mac)
{
    mac->instance = instance;
    mac->operation = MAC_OPERATION_IDLE;
    mac->pending_active_scan = false;
    mac->pending_energy_scan = false;
    mac->pending_transmit_beacon = false;
    mac->pending_transmit_data = false;
    mac->pending_transmit_oob_frame = false;
    mac->pending_waiting_for_data = false;
    mac->rx_on_when_idle = false;
    mac->beacons_enabled = false;
    mac->transmit_aborted = false;
#if NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS
    mac->delay_sleep = false;
#endif
    tasklet_ctor(instance, &mac->operation_task, &handle_operation_task, mac);
    timer_ctor(instance, &mac->mac_timer, &handle_mac_timer, mac);
    timer_ctor(instance, &mac->backoff_timer, &handle_backoff_timer, mac);
}

// --- private functions
