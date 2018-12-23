#include "core/mac/mac.h"
#include "core/common/code_utils.h"
#include "core/common/debug.h"
#include "core/common/encoding.h"
#include "core/common/instance.h"
#include "core/common/random.h"
// TODO: #include "core/crypto/aes_ccm.h"
// TODO: #include "core/crypto/sha256.h"

#if 0 // TODO:
static const uint8_t s_mode2_key[] = {0x78, 0x58, 0x16, 0x86, 0xfd, 0xb4, 0x58, 0x0f,
                                      0xb0, 0x92, 0x54, 0x6a, 0xec, 0xbd, 0x15, 0x66};

static const ns_ext_addr_t s_mode2_ext_addr = {
    {0x35, 0x06, 0xfe, 0xb8, 0x23, 0xd4, 0x87, 0x12},
};
#endif

static const ns_extended_panid_t s_extended_panid_init = {
    {0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xfe},
};

static const char s_network_name_init[] = "ns-thread";

// --- private functions declarations
static void
mac_process_transmit_security(mac_t *mac, mac_frame_t *frame, bool process_aes_ccm);

#if 0 // TODO:
static void
mac_generate_nonce(mac_t *mac,
                   const mac_ext_addr_t *addr,
                   uint32_t frame_counter,
                   uint8_t security_level,
                   uint8_t *nonce);

static ns_error_t
mac_process_receive_security(mac_t *mac,
                             mac_frame_t *frame,
                             const mac_addr_t *src_addr,
                             neighbor_t *neighbor);
#endif

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

#if 0
static bool
mac_should_send_beacon(mac_t *mac);
#endif

static void
mac_begin_transmit(mac_t *mac);

#if 0 // TODO:
static ns_error_t
mac_handle_mac_command(mac_t *mac, mac_frame_t *frame);
#endif

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

#if 0 // TODO:
static void
mac_log_frame_rx_failure(mac_t *mac, const mac_frame_t *frame, ns_error_t error);

static void
mac_log_frame_tx_failure(mac_t *mac, const mac_frame_t *frame, ns_error_t error);
#endif

static void
mac_log_beacon(mac_t *mac, const char *action_text, const mac_beacon_payload_t *beacon_payload);

#if NS_CONFIG_ENABLE_TIME_SYNC
static void
mac_process_time_ie(mac_t *mac, mac_frame_t *frame);

static uint8_t
mac_get_time_ie_offset(mac_t *mac, mac_frame_t *frame);
#endif

static const char *
mac_operation_to_string(mac_operation_t operation);

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
    timer_ctor(instance, &mac->receive_timer, &handle_receive_timer, mac);
    mac->short_addr = MAC_SHORT_ADDR_INVALID;
    mac->panid = MAC_PANID_BROADCAST;
    mac->pan_channel = NS_CONFIG_DEFAULT_CHANNEL;
    mac->radio_channel = NS_CONFIG_DEFAULT_CHANNEL;
    mac->radio_channel_acquisition_id = 0;
    mac_channel_mask_ctor(&mac->supported_channel_mask, NS_RADIO_SUPPORTED_CHANNELS);
    mac->beacon_sequence = random_get_uint8();
    mac->data_sequence = random_get_uint8();
    mac->csma_backoffs = 0;
    mac->transmit_retries = 0;
    mac->broadcast_transmit_count = 0;
    mac_channel_mask_ctor(&mac->scan_channel_mask, 0);
    mac->scan_duration = 0;
    mac->scan_channel = NS_RADIO_CHANNEL_MIN;
    mac->energy_scan_current_max_rssi = MAC_INVALID_RSSI_VALUE;
    mac->scan_context = NULL;
    mac->active_scan_handler = NULL;
    mac->pcap_callback = NULL;
    mac->pcap_callback_context = NULL;
#if NS_ENABLE_MAC_FILTER
    mac_filter_ctor(&mac->filter);
#endif // NS_ENABLE_MAC_FILTER
    mac->tx_frame = (mac_frame_t *)ns_plat_radio_get_transmit_buffer(instance);
    mac->oob_frame = NULL;
    mac->key_id_mode2_frame_counter = 0;
    mac->cca_sample_count = 0;
    mac->enabled = true;

    mac_ext_addr_generate_random(&mac->ext_addr);
    success_rate_tracker_reset(&mac->cca_success_rate_tracker);
    memset(&mac->counters, 0, sizeof(ns_mac_counters_t));
    memset(&mac->network_name, 0, sizeof(ns_network_name_t));

    ns_plat_radio_enable(instance);

    mac_set_extended_panid(mac, &s_extended_panid_init);
    mac_set_network_name(mac, s_network_name_init);
    mac_set_panid(mac, mac->panid);
    mac_set_ext_addr(mac, &mac->ext_addr);
    mac_set_short_addr(mac, mac->short_addr);
}

ns_error_t
mac_active_scan(mac_t *mac,
                uint32_t scan_channels,
                uint16_t scan_duration,
                active_scan_handler_func_t handler,
                void *context)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(mac->enabled, error = NS_ERROR_INVALID_STATE);
    VERIFY_OR_EXIT(!mac_is_active_scan_in_progress(mac) && !mac_is_energy_scan_in_progress(mac),
                   error = NS_ERROR_BUSY);

    mac->active_scan_handler = handler;

    if (scan_duration == 0) {
        scan_duration = MAC_SCAN_DURATION_DEFAULT;
    }

    mac_scan(mac, MAC_OPERATION_ACTIVE_SCAN, scan_channels, scan_duration, context);

exit:
    return error;
}

ns_error_t
mac_convert_beacon_to_active_scan_result(mac_t *mac,
                                         mac_frame_t *beacon_frame,
                                         ns_active_scan_result_t *result)
{
    ns_error_t error = NS_ERROR_NONE;
    mac_addr_t addr;
    mac_beacon_t *beacon = NULL;
    mac_beacon_payload_t *beacon_payload = NULL;
    uint8_t payload_length;

    memset(result, 0, sizeof(ns_active_scan_result_t));

    VERIFY_OR_EXIT(beacon_frame != NULL, error = NS_ERROR_INVALID_ARGS);

    VERIFY_OR_EXIT(mac_frame_get_type(beacon_frame) == MAC_FRAME_FCF_FRAME_BEACON, error = NS_ERROR_PARSE);
    SUCCESS_OR_EXIT(error = mac_frame_get_src_addr(beacon_frame, &addr));
    VERIFY_OR_EXIT(mac_addr_type_is_extended(&addr), error = NS_ERROR_PARSE);
    result->ext_addr = *(ns_ext_addr_t *)mac_addr_get_extended(&addr);

    mac_frame_get_src_panid(beacon_frame, &result->panid);
    result->channel = mac_frame_get_channel(beacon_frame);
    result->rssi = mac_frame_get_rssi(beacon_frame);
    result->lqi = mac_frame_get_lqi(beacon_frame);

    payload_length = mac_frame_get_payload_length(beacon_frame);

    beacon = (mac_beacon_t *)mac_frame_get_payload(beacon_frame);
    beacon_payload = (mac_beacon_payload_t *)mac_beacon_get_payload(beacon);

    if ((payload_length >= (sizeof(*beacon) + sizeof(*beacon_payload))) &&
        mac_beacon_is_valid(beacon) && mac_beacon_payload_is_valid(beacon_payload)) {
        result->version = mac_beacon_payload_get_protocol_version(beacon_payload);
        result->is_joinable = mac_beacon_payload_is_joining_permitted(beacon_payload);
        result->is_native = mac_beacon_payload_is_native(beacon_payload);
        memcpy(&result->network_name,
               mac_beacon_payload_get_network_name(beacon_payload),
               MAC_BEACON_PAYLOAD_NETWORK_NAME_SIZE);
        memcpy(&result->extended_panid,
               mac_beacon_payload_get_extended_panid(beacon_payload),
               MAC_BEACON_PAYLOAD_EXT_PANID_SIZE);
    }

    mac_log_beacon(mac, "received", beacon_payload);

exit:
    return error;
}

ns_error_t
mac_energy_scan(mac_t *mac,
                uint32_t scan_channels,
                uint16_t scan_duration,
                energy_scan_handler_func_t handler,
                void *context)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(mac->enabled, error = NS_ERROR_INVALID_STATE);
    VERIFY_OR_EXIT(!mac_is_active_scan_in_progress(mac) && !mac_is_energy_scan_in_progress(mac),
                   error = NS_ERROR_BUSY);

    mac->energy_scan_handler = handler;
    mac_scan(mac, MAC_OPERATION_ENERGY_SCAN, scan_channels, scan_duration, context);
exit:
    return error;
}

void
mac_energy_scan_done(mac_t *mac, int8_t rssi)
{
    mac_report_energy_scan_result(mac, rssi);
    mac_perform_energy_scan(mac);
}

bool
mac_is_beacon_enabled(mac_t *mac)
{
    return mac->beacons_enabled;
}

void
mac_set_beacon_enabled(mac_t *mac, bool enabled)
{
    mac->beacons_enabled = enabled;
}

bool
mac_get_rx_on_when_idle(mac_t *mac)
{
    return mac->rx_on_when_idle;
}

void
mac_set_rx_on_when_idle(mac_t *mac, bool rx_on_when_idle)
{
    VERIFY_OR_EXIT(mac->rx_on_when_idle != rx_on_when_idle);
    mac->rx_on_when_idle = rx_on_when_idle;

    // If the new value for `mac->rx_on_when_idle` is `true` (i.e., radio should
    // remain in rx while idle) we stop any ongoing or pending `waiting_for_data`
    // operation (since this operation only applies to sleepy devices).

    if (mac->rx_on_when_idle) {
        mac->pending_waiting_for_data = false;
        if (mac->operation == MAC_OPERATION_WAITING_FOR_DATA) {
            timer_milli_stop(&mac->receive_timer);
            mac_finish_operation(mac);
            mac_perform_next_operation(mac);
        }
    }

    mac_update_idle_mode(mac);

exit:
    return;
}

ns_error_t
mac_send_frame_request(mac_t *mac)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(mac->enabled, error = NS_ERROR_INVALID_STATE);
    VERIFY_OR_EXIT(!mac->pending_transmit_data && (mac->operation != MAC_OPERATION_TRANSMIT_DATA),
                   error = NS_ERROR_ALREADY);
    mac_start_operation(mac, MAC_OPERATION_TRANSMIT_DATA);
exit:
    return error;
}

ns_error_t
mac_send_out_of_band_frame_request(mac_t *mac, ns_radio_frame_t *oob_frame)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(mac->enabled, error = NS_ERROR_INVALID_STATE);
    VERIFY_OR_EXIT(mac->oob_frame == NULL, error = NS_ERROR_ALREADY);
    mac->oob_frame = (mac_frame_t *)oob_frame;
    mac_start_operation(mac, MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME);
exit:
    return error;
}

const mac_ext_addr_t *
mac_get_ext_addr(mac_t *mac)
{
    return &mac->ext_addr;
}

void
mac_set_ext_addr(mac_t *mac, const mac_ext_addr_t *ext_addr)
{
    ns_ext_addr_t addr;
    for (size_t i = 0; i < sizeof(addr); i++) {
        addr.m8[i] = ext_addr->m8[7 - i];
    }
    ns_plat_radio_set_extended_addr(mac->instance, &addr);
    mac->ext_addr = *(mac_ext_addr_t *)ext_addr;
}

mac_short_addr_t
mac_get_short_addr(mac_t *mac)
{
    return mac->short_addr;
}

ns_error_t
mac_set_short_addr(mac_t *mac, mac_short_addr_t short_addr)
{
    mac->short_addr = short_addr;
    ns_plat_radio_set_short_addr(mac->instance, short_addr);
    return NS_ERROR_NONE;
}

uint8_t
mac_get_pan_channel(mac_t *mac)
{
    return mac->pan_channel;
}

ns_error_t
mac_set_pan_channel(mac_t *mac, uint8_t channel)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(NS_RADIO_CHANNEL_MIN <= channel && channel <= NS_RADIO_CHANNEL_MAX,
                   error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(mac_channel_mask_contains_channel(&mac->supported_channel_mask, channel),
                   error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(mac->pan_channel != channel,
                   notifier_signal_if_first(instance_get_notifier(mac->instance), NS_CHANGED_THREAD_CHANNEL));

    mac->pan_channel = channel;
    success_rate_tracker_reset(&mac->cca_success_rate_tracker);

    VERIFY_OR_EXIT(!mac->radio_channel_acquisition_id);

    mac->radio_channel = mac->pan_channel;

    mac_update_idle_mode(mac);

    notifier_signal(instance_get_notifier(mac->instance), NS_CHANGED_THREAD_CHANNEL);

exit:
    return error;
}

uint8_t
mac_get_radio_channel(mac_t *mac)
{
    return mac->radio_channel;
}

ns_error_t
mac_set_radio_channel(mac_t *mac, uint16_t acquisition_id, uint8_t channel)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(NS_RADIO_CHANNEL_MIN <= channel && channel <= NS_RADIO_CHANNEL_MAX,
                   error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(mac_channel_mask_contains_channel(&mac->supported_channel_mask, channel),
                   error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(mac->radio_channel_acquisition_id && acquisition_id == mac->radio_channel_acquisition_id,
                   error = NS_ERROR_INVALID_STATE);

    mac->radio_channel = channel;

    mac_update_idle_mode(mac);

exit:
    return error;
}

ns_error_t
mac_acquired_radio_channel(mac_t *mac, uint16_t *acquisition_id)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(acquisition_id != NULL, error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(!mac->radio_channel_acquisition_id, error = NS_ERROR_INVALID_STATE);

    mac->radio_channel_acquisition_id = random_get_uint16_in_range(1, MAC_MAX_ACQUISITION_ID);

    *acquisition_id = mac->radio_channel_acquisition_id;

exit:
    return error;
}

ns_error_t
mac_release_radio_channel(mac_t *mac)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(mac->radio_channel_acquisition_id, error = NS_ERROR_INVALID_STATE);

    mac->radio_channel_acquisition_id = 0;
    mac->radio_channel = mac->pan_channel;

    mac_update_idle_mode(mac);

exit:
    return error;
}

const mac_channel_mask_t
mac_get_supported_channel_mask(mac_t *mac)
{
    return mac->supported_channel_mask;
}

void
mac_set_supported_channel_mask(mac_t *mac, const mac_channel_mask_t mask)
{
    mac_channel_mask_t new_mask = mask;

    mac_channel_mask_t supported_channel;
    mac_channel_mask_ctor(&supported_channel, NS_RADIO_SUPPORTED_CHANNELS);

    mac_channel_mask_intersect(&new_mask, &supported_channel);

    VERIFY_OR_EXIT(new_mask.mask != mac->supported_channel_mask.mask,
                   notifier_signal_if_first(instance_get_notifier(mac->instance),
                                            NS_CHANGED_SUPPORTED_CHANNEL_MASK));

    mac->supported_channel_mask = new_mask;

    notifier_signal(instance_get_notifier(mac->instance), NS_CHANGED_SUPPORTED_CHANNEL_MASK);

exit:
    return;
}

const char *
mac_get_network_name(mac_t *mac)
{
    return mac->network_name.m8;
}

ns_error_t
mac_set_network_name(mac_t *mac, const char *network_name)
{
    return mac_set_network_name_buf(mac, network_name, NS_NETWORK_NAME_MAX_SIZE + 1);
}

ns_error_t
mac_set_network_name_buf(mac_t *mac, const char *buffer, uint8_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t new_len = (uint8_t)(strnlen(buffer, length));

    VERIFY_OR_EXIT(new_len <= NS_NETWORK_NAME_MAX_SIZE, error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(new_len != strlen(mac->network_name.m8) ||
                   memcmp(mac->network_name.m8, buffer, new_len) != 0,
                   notifier_signal_if_first(instance_get_notifier(mac->instance),
                                            NS_CHANGED_THREAD_NETWORK_NAME));
    memcpy(mac->network_name.m8, buffer, new_len);
    mac->network_name.m8[new_len] = 0;
    notifier_signal(instance_get_notifier(mac->instance), NS_CHANGED_THREAD_NETWORK_NAME);

exit:
    return error;
}

uint16_t
mac_get_panid(mac_t *mac)
{
    return mac->panid;
}

ns_error_t
mac_set_panid(mac_t *mac, uint16_t panid)
{
    VERIFY_OR_EXIT(mac->panid != panid,
                   notifier_signal_if_first(instance_get_notifier(mac->instance), NS_CHANGED_THREAD_PANID));

    mac->panid = panid;
    ns_plat_radio_set_panid(mac->instance, mac->panid);

    notifier_signal(instance_get_notifier(mac->instance), NS_CHANGED_THREAD_PANID);

exit:
    return NS_ERROR_NONE;
}

const ns_extended_panid_t *
mac_get_extended_panid(mac_t *mac)
{
    return &mac->extended_panid;
}

ns_error_t
mac_set_extended_panid(mac_t *mac, const ns_extended_panid_t *extended_panid)
{
    VERIFY_OR_EXIT(memcmp(mac->extended_panid.m8, extended_panid->m8, sizeof(mac->extended_panid)) != 0,
                   notifier_signal_if_first(instance_get_notifier(mac->instance), NS_CHANGED_THREAD_EXT_PANID));

    mac->extended_panid = *extended_panid;
    notifier_signal(instance_get_notifier(mac->instance), NS_CHANGED_THREAD_EXT_PANID);

exit:
    return NS_ERROR_NONE;
}

#if NS_ENABLE_MAC_FILTER
mac_filter_t
mac_get_filter(mac_t *mac)
{
    return mac->filter;
}
#endif

void
mac_handle_received_frame(mac_t *mac, mac_frame_t *frame, ns_error_t error)
{
    // TODO:
}

void
mac_handle_transmit_started(mac_t *mac, ns_radio_frame_t *aframe)
{
    mac_frame_t *frame = (mac_frame_t *)aframe;
    if (mac_frame_get_ack_request(frame) &&
        !(ns_plat_radio_get_caps(mac->instance) & NS_RADIO_CAPS_ACK_TIMEOUT)) {
        timer_milli_start(&mac->mac_timer, MAC_ACK_TIMEOUT);
        ns_log_debg_mac("ack timer start");
    }
}

void
mac_handle_transmit_done(mac_t *mac, ns_radio_frame_t *frame, ns_radio_frame_t *ack_frame, ns_error_t error)
{
    // TODO:
}

bool
mac_is_active_scan_in_progress(mac_t *mac)
{
    return (mac->operation == MAC_OPERATION_ACTIVE_SCAN) || (mac->pending_active_scan);
}

bool
mac_is_energy_scan_in_progress(mac_t *mac)
{
    return (mac->operation == MAC_OPERATION_ENERGY_SCAN) || (mac->pending_energy_scan);
}

bool
mac_is_in_transmit_state(mac_t *mac)
{
    return (mac->operation == MAC_OPERATION_TRANSMIT_DATA) ||
           (mac->operation == MAC_OPERATION_TRANSMIT_BEACON) ||
           (mac->operation == MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME);
}

void
mac_set_pcap_callback(mac_t *mac, ns_link_pcap_callback_func_t pcap_callback, void *context)
{
    mac->pcap_callback = pcap_callback;
    mac->pcap_callback_context = context;
}

bool
mac_is_promiscuous(mac_t *mac)
{
    return ns_plat_radio_get_promiscuous(mac->instance);
}

void
mac_set_promiscuous(mac_t *mac, bool promiscuous)
{
    ns_plat_radio_set_promiscuous(mac->instance, promiscuous);
    mac_update_idle_mode(mac);
}

void
mac_reset_counters(mac_t *mac)
{
    memset(&mac->counters, 0, sizeof(mac->counters));
}

ns_mac_counters_t *
mac_get_counters(mac_t *mac)
{
    return &mac->counters;
}

int8_t
mac_get_noise_floor(mac_t *mac)
{
    return ns_plat_radio_get_receive_sensitivity(mac->instance);
}

bool
mac_radio_supports_csma_backoffs(mac_t *mac)
{
    return (ns_plat_radio_get_caps(mac->instance) & (NS_RADIO_CAPS_TRANSMIT_RETRIES | NS_RADIO_CAPS_CSMA_BACKOFF)) != 0;
}

bool
mac_radio_supports_retries(mac_t *mac)
{
    return (ns_plat_radio_get_caps(mac->instance) & NS_RADIO_CAPS_TRANSMIT_RETRIES) != 0;
}

uint16_t
mac_get_cca_failure_rate(mac_t *mac)
{
    return success_rate_tracker_get_failure_rate(&mac->cca_success_rate_tracker);
}

ns_error_t
mac_set_enabled(mac_t *mac, bool enabled)
{
    mac->enabled = enabled;
    return NS_ERROR_NONE;
}

bool
mac_is_enabled(mac_t *mac)
{
    return mac->enabled;
}

void
mac_process_transmit_aes_ccm(mac_t *mac, mac_frame_t *frame, const mac_ext_addr_t *ext_addr)
{
    // TODO:
}

// --- private functions
static void
mac_process_transmit_security(mac_t *mac, mac_frame_t *frame, bool process_aes_ccm)
{
    // TODO:
}

#if 0 // TODO:
static void
mac_generate_nonce(mac_t *mac,
                   const mac_ext_addr_t *addr,
                   uint32_t frame_counter,
                   uint8_t security_level,
                   uint8_t *nonce)
{
    // source address
    for (int i = 0; i < 8; i++) {
        nonce[i] = addr->m8[i];
    }

    nonce += 8;

    // frame_counter
    nonce[0] = (frame_counter >> 24) & 0xff;
    nonce[1] = (frame_counter >> 16) & 0xff;
    nonce[2] = (frame_counter >> 8) & 0xff;
    nonce[3] = (frame_counter >> 0) & 0xff;
    nonce += 4;

    // security level
    nonce[0] = security_level;
}

static ns_error_t
mac_process_receive_security(mac_t *mac,
                             mac_frame_t *frame,
                             const mac_addr_t *src_addr,
                             neighbor_t *neighbor)
{
    // TODO:
    return NS_ERROR_NONE;
}
#endif

static void
mac_update_idle_mode(mac_t *mac)
{
    VERIFY_OR_EXIT(mac->operation == MAC_OPERATION_IDLE);

    if (!mac->rx_on_when_idle && !timer_is_running(&mac->receive_timer) && !ns_plat_radio_get_promiscuous(mac->instance)) {
        if (mac_radio_sleep(mac) != NS_ERROR_INVALID_STATE) {
            ns_log_debg_mac("idle mode: radio sleeping");
            EXIT_NOW();
        }
        // If `mac_radio_sleep()` returns `NS_ERROR_INVALID_STATE`
        // indicating sleep is being delayed, continue to put
        // the radio in receive mode.
    }

    ns_log_debg_mac("idle mode: radio receiving on channel %d", mac->radio_channel);
    mac_radio_receive(mac, mac->radio_channel);

exit:
    return;
}

static void
mac_start_operation(mac_t *mac, mac_operation_t operation)
{
    if (operation != MAC_OPERATION_IDLE) {
        ns_log_debg_mac("request to start operation \"%s\"", mac_operation_to_string(operation));
    }
    switch (operation) {
    case MAC_OPERATION_IDLE:
        break;
    case MAC_OPERATION_ACTIVE_SCAN:
        mac->pending_active_scan = true;
        break;
    case MAC_OPERATION_ENERGY_SCAN:
        mac->pending_energy_scan = true;
        break;
    case MAC_OPERATION_TRANSMIT_BEACON:
        mac->pending_transmit_beacon = true;
        break;
    case MAC_OPERATION_TRANSMIT_DATA:
        mac->pending_transmit_data = true;
        break;
    case MAC_OPERATION_WAITING_FOR_DATA:
        mac->pending_waiting_for_data = true;
        break;
    case MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME:
        mac->pending_transmit_oob_frame = true;
        break;
    }
    if (mac->operation == MAC_OPERATION_IDLE) {
        tasklet_post(&mac->operation_task);
    }
}

static void
mac_finish_operation(mac_t *mac)
{
    ns_log_debg_mac("finishing operation \"%s\"", mac_operation_to_string(mac->operation));
    mac->operation = MAC_OPERATION_IDLE;
}

static void
mac_perform_next_operation(mac_t *mac)
{
    VERIFY_OR_EXIT(mac->operation == MAC_OPERATION_IDLE);
    if (!mac->enabled) {
        mac->pending_waiting_for_data = false;
        mac->pending_transmit_oob_frame = false;
        mac->pending_active_scan = false;
        mac->pending_energy_scan = false;
        mac->pending_transmit_beacon = false;
        mac->pending_transmit_data = false;
        mac->oob_frame = NULL;
        EXIT_NOW();
    }

    // `waiting_for_data` should be checked before any other pending
    // operations since radio should remain in receive mode after
    // a data poll ack indicating a pending frame from parent.
    if (mac->pending_waiting_for_data) {
        mac->pending_waiting_for_data = false;
        mac->operation = MAC_OPERATION_WAITING_FOR_DATA;
        mac_radio_receive(mac, mac->radio_channel);
    } else if (mac->pending_transmit_oob_frame) {
        mac->pending_transmit_oob_frame = false;
        mac->operation = MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME;
        mac_start_csma_backoff(mac);
    } else if (mac->pending_active_scan) {
        mac->pending_active_scan = false;
        mac->operation = MAC_OPERATION_ACTIVE_SCAN;
        mac_perform_active_scan(mac);
    } else if (mac->pending_energy_scan) {
        mac->pending_energy_scan = false;
        mac->operation = MAC_OPERATION_ENERGY_SCAN;
        mac_perform_energy_scan(mac);
    } else if (mac->pending_transmit_beacon) {
        mac->pending_transmit_beacon = false;
        mac->operation = MAC_OPERATION_TRANSMIT_BEACON;
        mac_start_csma_backoff(mac);
    } else if (mac->pending_transmit_data) {
        mac->pending_transmit_data = false;
        mac->operation = MAC_OPERATION_TRANSMIT_DATA;
        mac_start_csma_backoff(mac);
    } else {
        mac_update_idle_mode(mac);
    }

    if (mac->operation != MAC_OPERATION_IDLE) {
        ns_log_debg_mac("starting operation \"%s\"", mac_operation_to_string(mac->operation));
    }

exit:
    return;
}

static void
mac_send_beacon_request(mac_t *mac, mac_frame_t *frame)
{
    uint16_t fcf = MAC_FRAME_FCF_FRAME_MAC_CMD | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_NONE;
    mac_frame_init_mac_header(frame, fcf, MAC_FRAME_SEC_NONE);
    mac_frame_set_dst_panid(frame, MAC_SHORT_ADDR_BROADCAST);
    mac_frame_set_dst_addr_short(frame, MAC_SHORT_ADDR_BROADCAST);
    mac_frame_set_command_id(frame, MAC_FRAME_MAC_CMD_BEACON_REQUEST);
    ns_log_info_mac("sending beacon request");
}

static void
mac_send_beacon(mac_t *mac, mac_frame_t *frame)
{
    //TODO: uint8_t num_unsecure_ports;
    uint8_t beacon_length;
    uint16_t fcf;
    mac_beacon_t *beacon = NULL;
    mac_beacon_payload_t *beacon_payload = NULL;

    // initialize mac header
    fcf = MAC_FRAME_FCF_FRAME_BEACON | MAC_FRAME_FCF_DST_ADDR_NONE | MAC_FRAME_FCF_SRC_ADDR_EXT;
    mac_frame_init_mac_header(frame, fcf, MAC_FRAME_SEC_NONE);
    mac_frame_set_src_panid(frame, mac->panid);
    mac_frame_set_src_addr_ext(frame, &mac->ext_addr);

    // write payload
    beacon = (mac_beacon_t *)mac_frame_get_payload(frame);
    mac_beacon_init(beacon);
    beacon_length = sizeof(*beacon);

    beacon_payload = (mac_beacon_payload_t *)mac_beacon_get_payload(beacon);

#if 0 // TODO:
    if (key_manager_get_security_policy_flags(thread_netif_get_key_manager(mac->instance)) &
        NS_SECURITY_POLICY_BEACONS) {
        mac_beacon_payload_init(beacon_payload);
        // set the joining permitted flags
        ip6_filter_get_unsecure_ports(thread_netif_get_ip6_filter(mac->instance), &num_unsecure_ports);
        if (num_unsecure_ports) {
            mac_beacon_payload_set_joining_permitted(beacon_payload);
        } else {
            mac_beacon_payload_clear_joining_permitted(beacon_payload);
        }
        mac_beacon_payload_set_network_name(beacon_payload, &mac->network_name.m8);
        mac_beacon_payload_set_extended_panid(beacon_payload, &mac->extended_panid.m8);
        beacon_length += sizeof(*beacon_payload);
    }
#endif

    mac_frame_set_payload_length(frame, beacon_length);

    mac_log_beacon(mac, "sending", beacon_payload);
}

#if 0 // TODO:
static bool
mac_should_send_beacon(mac_t *mac)
{
    bool should_send = false;

    VERIFY_OR_EXIT(mac->enabled);

    should_send = mac_is_beacon_enabled(mac);

#if NS_CONFIG_ENABLE_BEACON_RSP_WHEN_JOINABLE
    if (!should_send) {
        // When `ENABLE_BEACON_RSP_WHEN_JOINABLE` feature is enabled,
        // the device should transmit IEEE 802.15.4 Beacons in response
        // to IEEE 802.15.4 Beacon Requests even while the device is not
        // router capable and detached (i.e., `mac_is_beacon_enabled()` is
        // false) but only if it is in joinable state (unsecure port
        // list is not empty).

        uint8_t num_unsecure_ports;

        // TODO: ip6_filter_get_unsecure_ports(thread_netif_get_ip6_filter(mac->instance), &num_unsecure_ports);
        should_send = (num_unsecure_ports != 0);
    }
#endif

exit:
    return should_send;
}
#endif

static void
mac_begin_transmit(mac_t *mac)
{
    ns_error_t error = NS_ERROR_NONE;
    bool apply_transmit_security = true;
    bool process_transmit_aes_ccm = true;
    mac_frame_t *send_frame = mac_get_operation_frame(mac);
#if NS_CONFIG_ENABLE_TIME_SYNC
    uint8_t time_ie_offset = 0;
#endif

    VERIFY_OR_EXIT(mac->enabled, error = NS_ERROR_ABORT);

    if (mac->csma_backoffs == 0 && mac->transmit_retries == 0 && mac->broadcast_transmit_count == 0) {
        switch (mac->operation) {
        case MAC_OPERATION_ACTIVE_SCAN:
            ns_plat_radio_set_panid(mac->instance, MAC_PANID_BROADCAST);
            mac_frame_set_channel(send_frame, mac->scan_channel);
            mac_send_beacon_request(mac, send_frame);
            mac_frame_set_sequence(send_frame, 0);
            mac_frame_set_max_csma_backoffs(send_frame, MAC_MAX_CSMA_BACKOFFS_DIRECT);
            mac_frame_set_max_frame_retries(send_frame, MAC_MAX_FRAME_RETRIES_DIRECT);
            break;
        case MAC_OPERATION_TRANSMIT_BEACON:
            mac_frame_set_channel(send_frame, mac->radio_channel);
            mac_send_beacon(mac, send_frame);
            mac_frame_set_sequence(send_frame, mac->beacon_sequence++);
            mac_frame_set_max_csma_backoffs(send_frame, MAC_MAX_CSMA_BACKOFFS_DIRECT);
            mac_frame_set_max_frame_retries(send_frame, MAC_MAX_FRAME_RETRIES_DIRECT);
            break;
        case MAC_OPERATION_TRANSMIT_DATA:
            mac_frame_set_channel(send_frame, mac->radio_channel);
            // TODO: SUCCESS_OR_EXIT(error = mesh_forwarder_handle_frame_request(thread_netif_get_mesh_forwarder(mac->instance), send_frame)); 
            // if the frame is marked as a retransmission, then data sequence number is alread set
            if (!mac_frame_is_a_retransmission(send_frame)) {
                mac_frame_set_sequence(send_frame, mac->data_sequence);
            }
            break;
        case MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME:
            apply_transmit_security = false;
            break;
        default:
            assert(false);
            break;
        }

#if NS_CONFIG_ENABLE_TIME_SYNC
#if 0 // TODO:
        time_ie_offset = mac_get_time_ie_offset(mac, send_frame);
        mac_frame_set_time_ie_offset(send_frame, time_ie_offset);
        if (time_ie_offset != 0) {
            // transmit security will be processed after time IE content is updated
            process_transmit_aes_ccm = false;
            mac_frame_set_time_sync_seq(send_frame, time_sync_get_network_time_offset(thread_netif_get_time_sync(mac->instance)));
        }
#endif
#endif // NS_CONFIG_ENABLE_TIME_SYNC

        if (apply_transmit_security) {
            // security processing
            mac_process_transmit_security(mac, send_frame, process_transmit_aes_ccm);
        }
    }

#if NS_CONFIG_DISABLE_CSMA_CA_ON_LAST_ATTEMPT
    if ((mac_frame_get_max_frame_retries(send_frame) > 0) &&
        (mac_frame_get_max_frame_retries(send_frame) <= mac->transmit_retries)) {
        mac_frame_set_csma_ca_enabled(send_frame, false);
    } else
#endif
    {
        mac_frame_set_csma_ca_enabled(send_frame, true);
    }

    error = mac_radio_receive(mac, mac_frame_get_channel(send_frame));
    assert(error == NS_ERROR_NONE);

    error = mac_radio_transmit(mac, send_frame);
    assert(error == NS_ERROR_NONE);

    if (mac->pcap_callback) {
        mac_frame_set_did_tx(send_frame, true);
        mac->pcap_callback(send_frame, mac->pcap_callback_context);
    }

exit:
    if (error != NS_ERROR_NONE) {
        // `mac_handle_transmit_done()` will be invoked from `mac->operation_task`
        // tasklet handler with error `NS_ERROR_ABORT`.
        mac->transmit_aborted = true;
        tasklet_post(&mac->operation_task);
    }
}

#if 0 // TODO:
static ns_error_t
mac_handle_mac_command(mac_t *mac, mac_frame_t *frame)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t command_id;

    mac_frame_get_command_id(frame, &command_id);

    switch (command_id) {
    case MAC_FRAME_MAC_CMD_BEACON_REQUEST:
        mac->counters.rx_beacon_request++;
        ns_log_info_mac("received beacon request");
        if (mac_should_send_beacon(mac)) {
            mac_start_operation(mac, MAC_OPERATION_TRANSMIT_BEACON);
        }
        EXIT_NOW(error = NS_ERROR_DROP);

    case MAC_FRAME_MAC_CMD_DATA_REQUEST:
        mac->counters.rx_data_poll++;
        break;

    default:
        mac->counters.rx_other++;
        break;
    }

exit:
    return error;
}
#endif

static mac_frame_t *
mac_get_operation_frame(mac_t *mac)
{
    mac_frame_t *frame = NULL;
    switch (mac->operation) {
    case MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME:
        frame = mac->oob_frame;
        break;
    default:
        frame = mac->tx_frame;
        break;
    }
    assert(frame != NULL);
    return frame;
}

static void
handle_mac_timer(void *timer)
{
    mac_t *mac = (mac_t *)((timer_t *)timer)->handler.context;
    mac_handle_mac_timer(mac);
}

static void
mac_handle_mac_timer(mac_t *mac)
{
    switch (mac->operation) {
    case MAC_OPERATION_ACTIVE_SCAN:
        mac_perform_active_scan(mac);
        break;
    case MAC_OPERATION_ENERGY_SCAN:
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
        timer_micro_stop(&mac->backoff_timer);
#else
        timer_milli_stop(&mac->backoff_timer);
#endif
        mac_energy_scan_done(mac, mac->energy_scan_current_max_rssi);
        break;
    case MAC_OPERATION_TRANSMIT_DATA:
        ns_log_debg_mac("ack timer fired");
        mac_radio_receive(mac, mac_frame_get_channel(mac->tx_frame));
        mac_handle_transmit_done(mac, mac->tx_frame, NULL, NS_ERROR_NO_ACK);
        break;
    default:
        assert(false);
        break;
    }
}

static void
handle_backoff_timer(void *timer)
{
    mac_t *mac = (mac_t *)((timer_t *)timer)->handler.context;
    mac_handle_backoff_timer(mac);
}

static void
mac_handle_backoff_timer(mac_t *mac)
{
    // The backoff timer serves two purposes:
    // (a) It is used to add CSMA backoff delay before a frame transmission.
    // (b) While performing Energy Scan, it is used to add delay between RSSI samples.

    if (mac->operation == MAC_OPERATION_ENERGY_SCAN) {
        mac_sample_rssi(mac);
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
        timer_micro_start_at(&mac->backoff_timer,
                             timer_get_firetime(&mac->backoff_timer),
                             MAC_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL);
#else
        timer_milli_start_at(&mac->backoff_timer,
                             timer_get_firetime(&mac->backoff_timer),
                             MAC_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL);
#endif
    } else {
        mac_begin_transmit(mac);
    }
}

static void
handle_receive_timer(void *timer)
{
    mac_t *mac = (mac_t *)((timer_t *)timer)->handler.context;
    mac_handle_receive_timer(mac);
}

static void
mac_handle_receive_timer(mac_t *mac)
{
    // `mac->receive_timer` is used for two purposes: (1) for data poll timeout
    // (i.e., waiting to receive a data frame after a data poll ack
    // indicating a pending frame from parent), and (2) for delaying sleep
    // when feature `NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS` is
    // enabled.

    if (mac->operation == MAC_OPERATION_WAITING_FOR_DATA) {
        ns_log_debg_mac("data poll timeout");
        mac_finish_operation(mac);
        // TODO: get_netif().get_mesh_forwarder().get_data_poll_manager().handle_poll_timeout();
        mac_perform_next_operation(mac);
    } else {
        ns_log_debg_mac("sleep delay timeout expired");
        mac_update_idle_mode(mac);
    }
}

static void
handle_operation_task(void *tasklet)
{
    mac_t *mac = (mac_t *)((tasklet_t *)tasklet)->handler.context;
    mac_handle_operation_task(mac);
}

static void
mac_handle_operation_task(mac_t *mac)
{
    // `mac->operation_task` tasklet is used for two separate purposes:
    // 1) To invoke `mac_handle_transmit_done()` from a tasklet with `NS_ERROR_ABORT` error.
    // 2) To perform a scheduled MAC operation.

    if (mac->transmit_aborted) {
        mac->transmit_aborted = false;
        mac_handle_transmit_done(mac, mac_get_operation_frame(mac), NULL, NS_ERROR_ABORT);
    } else {
        mac_perform_next_operation(mac);
    }
}

static void
mac_start_csma_backoff(mac_t *mac)
{
    uint32_t backoff_exponent = MAC_MIN_BE + mac->transmit_retries + mac->csma_backoffs;
    uint32_t backoff;
    bool should_receive;

    if (mac_radio_supports_csma_backoffs(mac)) {
        // If the radio supports CSMA back off logic, immediately schedule the send.
        mac_begin_transmit(mac);
        EXIT_NOW();
    }
#if NS_CONFIG_DISABLE_CSMA_CA_ON_LAST_ATTEMPT
    else if (mac->transmit_retries > 0) {
        mac_frame_t *send_frame = mac_get_operation_frame(mac);
        if ((mac_frame_get_max_frame_retries(send_frame) > 0) &&
            (mac_frame_get_max_frame_retries(send_frame) <= mac->transmit_retries)) {
            mac_begin_transmit(mac);
            EXIT_NOW();
        }
    }
#endif // NS_CONFIG_DISABLE_CSMA_CA_ON_LAST_ATTEMPT

    if (backoff_exponent > MAC_MAX_BE) {
        backoff_exponent = MAC_MAX_BE;
    }

    backoff = random_get_uint32_in_range(0, 1U << backoff_exponent);
    backoff *= ((uint32_t)(MAC_UNIT_BACKOFF_PERIOD) * NS_RADIO_SYMBOL_TIME);

    // Put the radio in either sleep or receive mode depending on
    // `mac->rx_on_when_idle` flag before starting the backoff timer.

    should_receive = (mac->rx_on_when_idle || ns_plat_radio_get_promiscuous(mac->instance));

    if (!should_receive) {
        if (mac_radio_sleep(mac) == NS_ERROR_INVALID_STATE) {
            // If `mac_radio_sleep()` returns `NS_ERROR_INVALID_STATE`
            // indicating sleep is being delayed, the radio should
            // be put in receive mode.
            should_receive = true;
        }
    }

    if (should_receive) {
        switch (mac->operation) {
        case MAC_OPERATION_ACTIVE_SCAN:
        case MAC_OPERATION_ENERGY_SCAN:
            mac_radio_receive(mac, mac->scan_channel);
            break;
        default:
            mac_radio_receive(mac, mac->radio_channel);
            break;
        }
    }

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_micro_start(&mac->backoff_timer, backoff);
#else
    timer_milli_start(&mac->backoff_timer, backoff / 1000UL);
#endif

exit:
    return;
}

static void
mac_scan(mac_t *mac,
         mac_operation_t scan_operation,
         uint32_t scan_channels,
         uint16_t scan_duration,
         void *context)
{
    mac->scan_context = context;
    mac->scan_duration = scan_duration;
    mac->scan_channel = MAC_CHANNEL_MASK_CHANNEL_ITERATOR_FIRST;

    if (scan_channels == 0) {
        scan_channels = NS_RADIO_SUPPORTED_CHANNELS;
    }

    mac_channel_mask_set_mask(&mac->scan_channel_mask, scan_channels);
    mac_channel_mask_intersect(&mac->scan_channel_mask, &mac->supported_channel_mask);
    mac_start_operation(mac, scan_operation);
}

static ns_error_t
mac_update_scan_channel(mac_t *mac)
{
    ns_error_t error;
    VERIFY_OR_EXIT(mac->enabled, error = NS_ERROR_ABORT);
    error = mac_channel_mask_get_next_channel(&mac->scan_channel_mask, &mac->scan_channel);
exit:
    return error;
}

static void
mac_perform_active_scan(mac_t *mac)
{
    if (mac_update_scan_channel(mac) == NS_ERROR_NONE) {
        // if there are more channels to scan, start CSMA backoff to send the beacon request
        mac_start_csma_backoff(mac);
    } else {
        ns_plat_radio_set_panid(mac->instance, mac->panid);
        mac_finish_operation(mac);
        mac->active_scan_handler(mac->scan_context, NULL);
        mac_perform_next_operation(mac);
    }
}

static void
mac_perform_energy_scan(mac_t *mac)
{
    ns_error_t error = NS_ERROR_NONE;
    SUCCESS_OR_EXIT(error = mac_update_scan_channel(mac));
    if (mac->scan_duration == 0) {
        while (true) {
            int8_t rssi;
            mac_radio_receive(mac, mac->scan_channel);
            rssi = ns_plat_radio_get_rssi(mac->instance);
            mac_report_energy_scan_result(mac, rssi);
            SUCCESS_OR_EXIT(error = mac_update_scan_channel(mac));
        }
    } else if ((ns_plat_radio_get_caps(mac->instance) & NS_RADIO_CAPS_ENERGY_SCAN) == 0) {
        mac_radio_receive(mac, mac->scan_channel);
        mac->energy_scan_current_max_rssi = MAC_INVALID_RSSI_VALUE;
        timer_milli_start(&mac->mac_timer, mac->scan_duration);
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
        timer_micro_start(&mac->backoff_timer, MAC_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL);
#else
        timer_milli_start(&mac->backoff_timer, MAC_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL);
#endif
        mac_sample_rssi(mac);
    } else {
        SUCCESS_OR_EXIT(error = ns_plat_radio_energy_scan(mac->instance, mac->scan_channel, mac->scan_duration));
    }
exit:
    if (error != NS_ERROR_NONE) {
        mac_finish_operation(mac);
        mac->energy_scan_handler(mac->scan_context, NULL);
        mac_perform_next_operation(mac);
    }
}

static void
mac_report_energy_scan_result(mac_t *mac, int8_t rssi)
{
    if (rssi != MAC_INVALID_RSSI_VALUE) {
        ns_energy_scan_result_t result;
        result.channel = mac->scan_channel;
        result.max_rssi = rssi;
        mac->energy_scan_handler(mac->scan_context, &result);
    }
}

static void
mac_sample_rssi(mac_t *mac)
{
    int8_t rssi;
    rssi = ns_plat_radio_get_rssi(mac->instance);
    if (rssi != MAC_INVALID_RSSI_VALUE) {
        if ((mac->energy_scan_current_max_rssi == MAC_INVALID_RSSI_VALUE) ||
            (rssi > mac->energy_scan_current_max_rssi)) {
            mac->energy_scan_current_max_rssi = rssi;
        }
    }
}

static ns_error_t
mac_radio_transmit(mac_t *mac, mac_frame_t *send_frame)
{
    ns_error_t error = NS_ERROR_NONE;
#if NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS
    if (!mac->rx_on_when_idle) {
        // cancel delay sleep timer
        timer_milli_stop(&mac->receive_timer);
        // delay sleep if we have another frame pending to transmit
        mac->delay_sleep = mac_frame_get_frame_pending(send_frame);
    }
#endif // NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS
    SUCCESS_OR_EXIT(error = ns_plat_radio_transmit(mac->instance, (ns_radio_frame_t *)send_frame));
exit:
    if (error != NS_ERROR_NONE) {
        ns_log_warn_mac("ns_plat_radio_transmit() failed with error %s", ns_error_to_string(error));
    }
    return error;
}

static ns_error_t
mac_radio_receive(mac_t *mac, uint8_t channel)
{
    ns_error_t error = NS_ERROR_NONE;
#if NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS
    if (!mac->rx_on_when_idle) {
        // cancel delay sleep timer
        timer_milli_stop(&mac->receive_timer);
    }
#endif
    SUCCESS_OR_EXIT(error = ns_plat_radio_receive(mac->instance, channel));
exit:
    if (error != NS_ERROR_NONE) {
        ns_log_warn_mac("ns_plat_radio_receive() failed with error %s", ns_error_to_string(error));
    }
    return error;
}

static ns_error_t
mac_radio_sleep(mac_t *mac)
{
    ns_error_t error = NS_ERROR_NONE;
#if NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS
    if (mac->delay_sleep) {
        ns_log_debg_mac("delaying sleep waiting for frame rx/tx");
        timer_milli_start(&mac->receive_timer, MAC_SLEEP_DELAY);
        mac->delay_sleep = false;
        // If sleep is delayed, `NS_ERROR_INVALID_STATE` is
        // returned to inform the caller to put/keep the
        // radio in receive mode.
        EXIT_NOW(error = NS_ERROR_INVALID_STATE);
    }
#endif // NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS

    error = ns_plat_radio_sleep(mac->instance);

    VERIFY_OR_EXIT(error != NS_ERROR_NONE);

    ns_log_warn_mac("ns_plat_radio_sleep() failed with error %s", ns_error_to_string(error));

exit:
    return error;
}

#if (NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO) && (NS_CONFIG_LOG_MAC == 1)
#if 0 // TODO:
static void
mac_log_frame_rx_failure(mac_t *mac, const mac_frame_t *frame, ns_error_t error)
{
    ns_log_level_t log_level;

    switch (error) {
    case NS_ERROR_ABORT:
    case NS_ERROR_NO_FRAME_RECEIVED:
    case NS_ERROR_DESTINATION_ADDRESS_FILTERED:
        log_level = NS_LOG_LEVEL_DEBG;
        break;
    default:
        log_level = NS_LOG_LEVEL_INFO;
        break;
    }

    if (frame == NULL) {
        ns_log_mac(log_level, "frame rx failed, error:%s", ns_error_to_string(error));
    } else {
        ns_log_mac(log_level, "frame rx failed, error:%s, %s", ns_error_to_string(error),
                   string_as_c_string(mac_frame_to_info_string((mac_frame_t *)frame)));
    }
}

static void
mac_log_frame_tx_failure(mac_t *mac, const mac_frame_t *frame, ns_error_t error)
{
    ns_log_info_mac("frame tx failed, error: %s, retries:%d/%d, %s", ns_error_to_string(error),
                    mac->transmit_retries, mac_frame_get_max_frame_retries((mac_frame_t *)frame),
                    string_as_c_string(mac_frame_to_info_string((mac_frame_t *)frame)));
}
#endif

static void
mac_log_beacon(mac_t *mac, const char *action_text, const mac_beacon_payload_t *beacon_payload)
{
    (void)mac;
    ns_log_info_mac("%s beacon, %s", action_text,
                    string_as_c_string(mac_beacon_payload_to_info_string((mac_beacon_payload_t *)beacon_payload)));
}

#else // #if (NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO) && (NS_CONFIG_LOG_MAC == 1)
#if 0 // TODO:
static void
mac_log_frame_rx_failure(mac_t *, const mac_frame_t *, ns_error_t)
{
}

static void
mac_log_frame_tx_failure(mac_t *, const mac_frame_t *, ns_error_t)
{
}
#endif
static void
mac_log_beacon(mac_t *, const char *, const mac_beacon_payload_t *)
{
}
#endif // #if (NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO) && (NS_CONFIG_LOG_MAC == 1)

#if NS_CONFIG_ENABLE_TIME_SYNC
static void
mac_process_time_ie(mac_t *mac, mac_frame_t *frame)
{
    mac_time_ie_t *time_ie = (time_ie_t *)mac_frame_get_time_ie(frame);
    VERIFY_OR_EXIT(time_ie != NULL);
    mac_frame_set_network_time_offset(frame, (int64_t)(mac_time_ie_get_time(time_ie)) - (int64_t)(mac_frame_get_time_stamp(frame)));
    mac_frame_set_time_sync_seq(frame, mac_time_ie_get_sequence(time_ie));
exit:
    return;
}

static uint8_t
mac_get_time_ie_offset(mac_t *mac, mac_frame_t *frame)
{
    uint8_t offset = 0;
    uint8_t *base = mac_frame_get_psdu(frame);
    uint8_t *cur = NULL;

    cur = mac_frame_get_time_ie(frame);
    VERIFY_OR_EXIT(cur != NULL);

    cur += sizeof(mac_vendor_ie_header_t);
    offset = (uint8_t)(cur - base);

exit:
    return offset;
}
#endif

static const char *
mac_operation_to_string(mac_operation_t operation)
{
    const char *retval = "";
    switch (operation) {
    case MAC_OPERATION_IDLE:
        retval = "idle";
        break;
    case MAC_OPERATION_ACTIVE_SCAN:
        retval = "active-scan";
        break;
    case MAC_OPERATION_ENERGY_SCAN:
        retval = "energy-scan";
        break;
    case MAC_OPERATION_TRANSMIT_BEACON:
        retval = "transmit-beacon";
        break;
    case MAC_OPERATION_TRANSMIT_DATA:
        retval = "transmit-data";
        break;
    case MAC_OPERATION_WAITING_FOR_DATA:
        retval = "waiting-for-data";
        break;
    case MAC_OPERATION_TRANSMIT_OUT_OF_BAND_FRAME:
        retval = "transmit-oob-frame";
        break;
    }
    return retval;
}
