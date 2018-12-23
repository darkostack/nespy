#include "core/common/debug.h"
#include "core/common/instance.h"
#include "core/common/random.h"
#include "core/mac/link_raw.h"
#include "core/mac/mac.h"
#include <string.h>

#if NS_RADIO || NS_ENABLE_RAW_LINK_API

// --- private functions declarations
static void
handle_operation_task(void *tasklet);

static void
link_raw_handle_operation_task(mac_link_raw_t *link_raw);

static void
link_raw_transmit_now(mac_link_raw_t *link_raw);

static void
link_raw_start_transmit(mac_link_raw_t *link_raw);

static void
handle_timer(void *timer);

static void
link_raw_handle_timer(mac_link_raw_t *link_raw);

#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
static void
link_raw_start_csma_backoff(mac_link_raw_t *link_raw);
#endif

#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
static void
link_raw_handle_energy_scan_timer(mac_link_raw_t *link_raw);
#endif

// --- mac link raw functions
void
mac_link_raw_ctor(void *instance, mac_link_raw_t *link_raw)
{
    link_raw->instance = instance;
    tasklet_ctor(instance, &link_raw->operation_task, &handle_operation_task, (void *)link_raw);
    link_raw->pending_transmit_data = false;
#if NS_MAC_LINK_RAW_TIMER_REQUIRED
    timer_ctor(instance, &link_raw->timer, &handle_timer, (void *)link_raw);
    link_raw->timer_reason = MAC_LINK_RAW_TIMER_REASON_NONE;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_ctor(instance, &link_raw->timer_micro, &handle_timer, (void *)link_raw);
#else
    timer_ctor(instance, &link_raw->energy_scan_timer, &handle_timer, (void *)link_raw);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
#endif // NS_MAC_LINK_RAW_TIMER_REQUIRED
#if NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
    link_raw->transmit_retries = 0;
#endif // NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
    link_raw->receive_channel = NS_CONFIG_DEFAULT_CHANNEL;
    link_raw->receive_done_callback = NULL;
    link_raw->transmit_done_callback = NULL;
    link_raw->energy_scan_done_callback = NULL;
    link_raw->transmit_frame = (mac_frame_t *)ns_plat_radio_get_transmit_buffer(instance);
    link_raw->radio_caps = ns_plat_radio_get_caps(instance);
}

bool
mac_link_raw_is_enabled(mac_link_raw_t *link_raw)
{
    return link_raw->enabled;
}

ns_error_t
mac_link_raw_set_enabled(mac_link_raw_t *link_raw, bool enabled)
{
    ns_error_t error = NS_ERROR_NONE;

    ns_log_info_plat("link_raw enabled=%d", enabled ? 1 : 0);

    // TODO:
#if 0
#if NS_MTD || NS_FTD
    VERIFY_OR_EXIT(thread_netif_is_up(link_raw->instance), error = NS_ERROR_INVALID_STATE);
#endif
#endif

    if (enabled) {
        ns_plat_radio_enable(link_raw->instance);
    } else {
        ns_plat_radio_disable(link_raw->instance);
    }

    link_raw->enabled = enabled;

    // TODO:
#if 0
#if NS_MTD || NS_FTD
exit:
#endif
#endif
    return error;
}

uint16_t
mac_link_raw_get_panid(mac_link_raw_t *link_raw)
{
    return link_raw->panid;
}

ns_error_t
mac_link_raw_set_panid(mac_link_raw_t *link_raw, uint16_t panid)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(mac_link_raw_is_enabled(link_raw), error = NS_ERROR_INVALID_STATE);
    ns_plat_radio_set_panid(link_raw->instance, panid);
    link_raw->panid = panid;
exit:
    return error;
}

uint8_t
mac_link_raw_get_channel(mac_link_raw_t *link_raw)
{
    return link_raw->receive_channel;
}

ns_error_t
mac_link_raw_set_channel(mac_link_raw_t *link_raw, uint8_t channel)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(mac_link_raw_is_enabled(link_raw), error = NS_ERROR_INVALID_STATE);
    link_raw->receive_channel = channel;
exit:
    return error;
}

ns_ext_addr_t *
mac_link_raw_get_ext_addr(mac_link_raw_t *link_raw)
{
    return &link_raw->ext_addr;
}

ns_error_t
mac_link_raw_set_ext_addr(mac_link_raw_t *link_raw, ns_ext_addr_t ext_addr)
{
    ns_ext_addr_t addr;
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(mac_link_raw_is_enabled(link_raw), error = NS_ERROR_INVALID_STATE);
    for (size_t i = 0; i < sizeof(addr); i++) {
        addr.m8[i] = ext_addr.m8[7 - i];
    }
    ns_plat_radio_set_extended_addr(link_raw->instance, &addr);
    link_raw->ext_addr = ext_addr;
exit:
    return error;
}

uint16_t
mac_link_raw_get_short_addr(mac_link_raw_t *link_raw)
{
    return link_raw->short_addr;
}

ns_error_t
mac_link_raw_set_short_addr(mac_link_raw_t *link_raw, uint16_t short_addr)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(mac_link_raw_is_enabled(link_raw), error = NS_ERROR_INVALID_STATE);
    ns_plat_radio_set_short_addr(link_raw->instance, short_addr);
    link_raw->short_addr = short_addr;
exit:
    return error;
}

ns_radio_caps_t
mac_link_raw_get_caps(mac_link_raw_t *link_raw)
{
    ns_radio_caps_t radio_caps = link_raw->radio_caps;
#if NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT
    radio_caps |= NS_RADIO_CAPS_ACK_TIMEOUT;
#endif
#if NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
    radio_caps |= NS_RADIO_CAPS_TRANSMIT_RETRIES;
#endif
#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
    radio_caps |= NS_RADIO_CAPS_CSMA_BACKOFF;
#endif
#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
    radio_caps |= NS_RADIO_CAPS_ENERGY_SCAN;
#endif
    return radio_caps;
}

ns_error_t
mac_link_raw_receive(mac_link_raw_t *link_raw, ns_link_raw_receive_done_func_t callback)
{
    ns_error_t error = NS_ERROR_NONE;
    if (link_raw->enabled) {
        link_raw->receive_done_callback = callback;
        error = ns_plat_radio_receive(link_raw->instance, link_raw->receive_channel);
    }
    return error;
}

void
mac_link_raw_invoke_receive_done(mac_link_raw_t *link_raw, ns_radio_frame_t *frame, ns_error_t error)
{
    if (link_raw->receive_done_callback) {
        if (error == NS_ERROR_NONE) {
            ns_log_info_plat("link raw invoke receive done (%d bytes)", frame->length);
            link_raw->receive_done_callback(link_raw->instance, frame, error);
        } else {
            ns_log_warn_plat("link raw invoke receive done (err=0x%x)", error);
        }
    }
}

ns_error_t
mac_link_raw_transmit(mac_link_raw_t *link_raw, ns_link_raw_transmit_done_func_t callback)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(link_raw->enabled, error = NS_ERROR_INVALID_STATE);

    link_raw->transmit_done_callback = callback;

    mac_frame_set_csma_ca_enabled(link_raw->transmit_frame, true);

#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
    if ((link_raw->radio_caps & NS_RADIO_CAPS_CSMA_BACKOFF) == 0) {
        link_raw->csma_backoffs = 0;
    }
#endif

#if NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
    if ((link_raw->radio_caps & NS_RADIO_CAPS_TRANSMIT_RETRIES) == 0) {
        link_raw->transmit_retries = 0;
    }
#endif

    link_raw_start_transmit(link_raw);

exit:
    return error;
}

void
mac_link_raw_invoke_transmit_done(mac_link_raw_t *link_raw,
                                  ns_radio_frame_t *frame,
                                  ns_radio_frame_t *ack_frame,
                                  ns_error_t error)
{
    assert(frame == link_raw->transmit_frame);

    if (error == NS_ERROR_NONE) {
        ns_log_debg_plat("link raw transmit done: %s", ns_error_to_string(error));
    } else {
        ns_log_warn_plat("link raw transmit done: %s", ns_error_to_string(error));
    }

#if NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT
    if ((link_raw->radio_caps & NS_RADIO_CAPS_ACK_TIMEOUT) == 0) {
        timer_milli_stop(&link_raw->timer);
    }
#endif // NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT

#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
    if ((link_raw->radio_caps & NS_RADIO_CAPS_CSMA_BACKOFF) == 0) {
        if (error == NS_ERROR_CHANNEL_ACCESS_FAILURE) {
            link_raw->csma_backoffs++;
            if (link_raw->csma_backoffs < frame->info.tx_info.max_csma_backoffs) {
                EXIT_NOW(link_raw_start_transmit(link_raw));
            }
        } else {
            link_raw->csma_backoffs = 0;
        }
    }
#endif // NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF

#if NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
    if ((link_raw->radio_caps & NS_RADIO_CAPS_TRANSMIT_RETRIES) == 0) {
        if (error != NS_ERROR_NONE) {
            if (link_raw->transmit_retries < frame->info.tx_info.max_frame_retries) {
                link_raw->transmit_retries++;
#if NS_CONFIG_DISABLE_CSMA_CA_ON_LAST_ATTEMPT
                if ((mac_frame_get_max_frame_retries(link_raw->transmit_frame) > 0) &&
                    (mac_frame_get_max_frame_retries(link_raw->transmit_frame) <= link_raw->transmit_retries)) {
                    mac_frame_set_csma_ca_enabled(link_raw->transmit_frame, false);
                }
#endif
                EXIT_NOW(link_raw_start_transmit(link_raw));
            }
        }
    }
#endif // NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT

    // transition back to receive state on previous channel
    ns_plat_radio_receive(link_raw->instance, link_raw->receive_channel);

    VERIFY_OR_EXIT(link_raw->transmit_done_callback != NULL);

    switch (error) {
    case NS_ERROR_NONE:
    case NS_ERROR_NO_ACK:
    case NS_ERROR_CHANNEL_ACCESS_FAILURE:
    case NS_ERROR_ABORT:
        break;
    default:
        error = NS_ERROR_ABORT;
        break;
    }

    link_raw->transmit_done_callback(link_raw->instance, frame, ack_frame, error);

exit:
    return;
}

ns_error_t
mac_link_raw_energy_scan(mac_link_raw_t *link_raw,
                         uint8_t scan_channel,
                         uint16_t scan_duration,
                         ns_link_raw_energy_scan_done_func_t callback)
{
    ns_error_t error = NS_ERROR_INVALID_STATE;

    if (link_raw->enabled) {
        link_raw->energy_scan_done_callback = callback;
        if (ns_plat_radio_get_caps(link_raw->instance) & NS_RADIO_CAPS_ENERGY_SCAN) {
            // do the hw offloaded energy scan
            error = ns_plat_radio_energy_scan(link_raw->instance, scan_channel, scan_duration);
        }
#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
        else {
            // start listening on the scan channel
            ns_plat_radio_receive(link_raw->instance, scan_channel);

            // reset the RSSI value and start scanning
            link_raw->energy_scan_rssi = MAC_LINK_RAW_INVALID_RSSI_VALUE;
            link_raw->timer_reason = MAC_LINK_RAW_TIMER_REASON_ENERGY_SCAN_COMPLETE;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
            timer_micro_start(&link_raw->timer_micro, 0);
#else
            timer_milli_start(&link_raw->energy_scan_timer, 0);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
            timer_milli_start(&link_raw->timer, scan_duration);
        }
#endif // NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
    }

    return error;
}

void
mac_link_raw_invoke_energy_scan_done(mac_link_raw_t *link_raw, int8_t energy_scan_max_rssi)
{
    if (mac_link_raw_is_enabled(link_raw) && link_raw->energy_scan_done_callback) {
        link_raw->energy_scan_done_callback(link_raw->instance, energy_scan_max_rssi);
        link_raw->energy_scan_done_callback = NULL;
    }
}

void
mac_link_raw_transmit_started(mac_link_raw_t *link_raw, ns_radio_frame_t *frame)
{
#if NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT
    // If we are implementing the ACK timeout logic, start a timer here (if ACK request)
    // to fire if we don't get a transmit done callback in time.
    if ((link_raw->radio_caps & NS_RADIO_CAPS_ACK_TIMEOUT) == 0 &&
        mac_frame_get_ack_request((mac_frame_t *)frame)) {
        ns_log_debg_plat("link raw starting ack timeout timer");
        link_raw->timer_reason = MAC_LINK_RAW_TIMER_REASON_ACK_TIMEOUT;
        timer_milli_start(&link_raw->timer, MAC_ACK_TIMEOUT);
    }
#else
    (void)frame;
#endif // NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT
}

// --- private functions
static void
handle_operation_task(void *tasklet)
{
    mac_link_raw_t *link_raw = (mac_link_raw_t *)((tasklet_t *)tasklet)->handler.context;
    link_raw_handle_operation_task(link_raw);
}

static void
link_raw_handle_operation_task(mac_link_raw_t *link_raw)
{
    if (link_raw->pending_transmit_data) {
        link_raw->pending_transmit_data = false;
        link_raw_transmit_now(link_raw);
    }
}

static void
link_raw_transmit_now(mac_link_raw_t *link_raw)
{
    ns_error_t error = ns_plat_radio_transmit(link_raw->instance, link_raw->transmit_frame);
    if (error != NS_ERROR_NONE) {
        mac_link_raw_invoke_transmit_done(link_raw, link_raw->transmit_frame, NULL, error);
    }
}

static void
link_raw_start_transmit(mac_link_raw_t *link_raw)
{
#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
    if ((link_raw->radio_caps & NS_RADIO_CAPS_CSMA_BACKOFF) == 0 &&
        mac_frame_is_csma_ca_enabled(link_raw->transmit_frame)) {
        link_raw_start_csma_backoff(link_raw);
    } else
#endif
    {
        link_raw->pending_transmit_data = true;
        tasklet_post(&link_raw->operation_task);
    }
}

#if NS_MAC_LINK_RAW_TIMER_REQUIRED
static void
handle_timer(void *timer)
{
    mac_link_raw_t *link_raw = (mac_link_raw_t *)((timer_t *)timer)->handler.context;

#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
    // energy scan uses a different timer for adding delay between RSSI samples
    if (timer != &link_raw->timer && link_raw->timer_reason == MAC_LINK_RAW_TIMER_REASON_ENERGY_SCAN_COMPLETE) {
        link_raw_handle_energy_scan_timer(link_raw);
    } else
#endif
    {
        link_raw_handle_timer(link_raw);
    }
}

static void
link_raw_handle_timer(mac_link_raw_t *link_raw)
{
    mac_link_raw_timer_reason_t timer_reason = link_raw->timer_reason;
    link_raw->timer_reason = MAC_LINK_RAW_TIMER_REASON_NONE;

    switch (timer_reason) {
#if NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT
    case MAC_LINK_RAW_TIMER_REASON_ACK_TIMEOUT:
    {
        // transition back to receive state on previous channel
        ns_plat_radio_receive(link_raw->instance, link_raw->receive_channel);

        // invoke completion callback for transmit
        mac_link_raw_invoke_transmit_done(link_raw, link_raw->transmit_frame, NULL, NS_ERROR_NO_ACK);
        break;
    }
#endif // NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT

#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
    case MAC_LINK_RAW_TIMER_REASON_CSMA_BACKOFF_COMPLETE:
    {
        link_raw_transmit_now(link_raw);
        break;
    }
#endif // NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF

#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
    case MAC_LINK_RAW_TIMER_REASON_ENERGY_SCAN_COMPLETE:
    {
        // invoke completion callback for the energy scan
        mac_link_raw_invoke_energy_scan_done(link_raw, link_raw->energy_scan_rssi);
        break;
    }
#endif // NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN

    default:
        assert(false);
        break;
    }
}
#endif // NS_MAC_LINK_RAW_TIMER_REQUIRED

#if NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
static void
link_raw_start_csma_backoff(mac_link_raw_t *link_raw)
{
    uint32_t backoff_exponent = MAC_MIN_BE + link_raw->transmit_retries + link_raw->csma_backoffs;
    uint32_t backoff;
    if (backoff_exponent > MAC_MAX_BE) {
        backoff_exponent = MAC_MAX_BE;
    }

    backoff = random_get_uint32_in_range(0, 1U << backoff_exponent);
    backoff *= (uint32_t)MAC_UNIT_BACKOFF_PERIOD * NS_RADIO_SYMBOL_TIME;

    ns_log_debg_plat("link raw starting retransmit timeout timer (%d ms)", backoff);
    link_raw->timer_reason = MAC_LINK_RAW_TIMER_REASON_CSMA_BACKOFF_COMPLETE;

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_micro_start(&link_raw->timer_micro, backoff);
#else
    timer_milli_start(&link_raw->timer, backoff / 1000UL);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
}
#endif // NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF

#if NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
static void
link_raw_handle_energy_scan_timer(mac_link_raw_t *link_raw)
{
    // only process if we are still energy scanning
    if (timer_is_running(&link_raw->timer) &&
        link_raw->timer_reason == MAC_LINK_RAW_TIMER_REASON_ENERGY_SCAN_COMPLETE) {
        int8_t rssi = ns_plat_radio_get_rssi(link_raw->instance);
        // only apply RSSI if it was a valid value
        if (rssi != MAC_LINK_RAW_INVALID_RSSI_VALUE) {
            if ((link_raw->energy_scan_rssi == MAC_LINK_RAW_INVALID_RSSI_VALUE) ||
                (rssi > link_raw->energy_scan_rssi)) {
                link_raw->energy_scan_rssi = rssi;
            }
        }

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
        timer_micro_start(&link_raw->timer_micro, MAC_LINK_RAW_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL);
#else
        timer_milli_start(&link_raw->energy_scan_timer, MAC_LINK_RAW_ENERGY_SCAN_RSSI_SAMPLE_INTERVAL);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    }
}
#endif // NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN

#if NS_RADIO
void
ns_plat_radio_receive_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_error_t error)
{
    mac_link_raw_invoke_receive_done(&((instance_t *)instance)->link_raw, frame, error);
}

void
ns_plat_radio_tx_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_radio_frame_t *ack_frame, ns_error_t error)
{
    mac_link_raw_invoke_transmit_done(&((instance_t *)instance)->link_raw, frame, ack_frame, error);
}

void
ns_plat_radio_tx_started(ns_instance_t instance, ns_radio_frame_t *frame)
{
    mac_link_raw_transmit_started(&((instance_t *)instance)->link_raw, frame);
}

void
ns_plat_radio_energy_scan_done(ns_instance_t instance, int8_t energy_scan_max_rssi)
{
    VERIFY_OR_EXIT(((instance_t *)instance)->is_initialized);
    mac_link_raw_invoke_energy_scan_done(&((instance_t *)instance)->link_raw, energy_scan_max_rssi);
exit:
    return;
}

#if NS_CONFIG_HEADER_IE_SUPPORT
void
ns_plat_radio_frame_updated(ns_instance_t instance, ns_radio_frame_t *frame)
{
    // Note: for now this functionality is not supported in Radio only mode.
    (void)instance;
    (void)frame;
}
#endif // NS_CONFIG_HEADER_IE_SUPPORT
#endif // NS_RADIO

#endif // #if NS_RADIO || NS_ENABLE_RAW_LINK_API
