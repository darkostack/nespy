#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/random.h"
#include "ns/sys/core/mac/mac_frame.h"

// --- MAC extended address functions
void
mac_ext_addr_gen_random(ext_addr_t *ext_addr)
{

}

bool
mac_ext_addr_is_group(ext_addr_t *ext_addr);

void
mac_ext_addr_set_group(ext_addr_t *ext_addr, bool group);

void
mac_ext_addr_toggle_group(ext_addr_t *ext_addr);

bool
mac_ext_addr_is_local(ext_addr_t *ext_addr);

void
mac_ext_addr_set_local(ext_addr_t *ext_addr, bool local);

void
mac_ext_addr_toggle_local(ext_addr_t *ext_addr);

bool
mac_ext_addr_is_equal(ext_addr_t *ext_addr1, ext_addr_t *ext_addr2);

// --- MAC address functions
void
mac_addr_make_new(mac_addr_t *mac_addr);

mac_addr_type_t
mac_addr_get_type(mac_addr_t *mac_addr);

bool
mac_addr_type_is_none(mac_addr_t *mac_addr);

bool
mac_addr_type_is_short(mac_addr_t *mac_addr);

bool
mac_addr_type_is_extended(mac_addr_t *mac_addr);

short_addr_t
mac_addr_get_short(mac_addr_t *mac_addr);

ext_addr_t
mac_addr_get_extended(mac_addr_t *mac_addr);

void
mac_addr_set_none(mac_addr_t *mac_addr);

void
mac_addr_set_short(mac_addr_t *mac_addr, short_addr_t *short_addr);

void
mac_addr_set_extended(mac_addr_t *mac_addr, ext_addr_t *ext_addr);

void
mac_addr_set_extended_from_buf(mac_addr_t *mac_addr, const uint8_t *buf, bool reverse);

void
mac_addr_is_broadcast(mac_addr_t *mac_addr);

void
mac_addr_is_short_addr_invalid(mac_addr_t *mac_addr);

// --- MAC header ie functions
void
mac_header_ie_init(header_ie_t *header_ie);

uint16_t
mac_header_ie_get_id(header_ie_t *header_ie);

void
mac_header_ie_set_id(header_ie_t *header_ie);

void
mac_header_ie_get_length(header_ie_t *header_ie);

void
mac_header_ie_set_length(header_ie_t *header_ie);

// --- MAC frame functions
ns_error_t
mac_frame_init_mac_header(mac_frame_t *frame, uint16_t fcf, uint8_t sec_ctl);

ns_error_t
mac_frame_validate_psdu(mac_frame_t *frame);

uint8_t
mac_frame_get_type(mac_frame_t *frame);

uint16_t
mac_frame_get_version(mac_frame_t *frame);

bool
mac_frame_get_security_enabled(mac_frame_t *frame);

bool
mac_frame_get_frame_pending(mac_frame_t *frame);

void
mac_frame_set_frame_pending(mac_frame_t *frame, bool frame_pending);

bool
mac_frame_get_ack_request(mac_frame_t *frame);

void
mac_frame_set_ack_request(mac_frame_t *frame, bool ack_request);

bool
mac_frame_is_ie_present(mac_frame_t *frame);

uint8_t
mac_frame_get_sequence(mac_frame_t *frame);

void
mac_frame_set_sequence(mac_frame_t *frame, uint8_t sequence);

ns_error_t
mac_frame_get_dst_panid(mac_frame_t *frame, panid_t *panid);

ns_error_t
mac_frame_set_dst_panid(mac_frame_t *frame, panid_t panid);

ns_error_t
mac_frame_get_dst_addr(mac_frame_t *frame, mac_addr_t *mac_addr);

ns_error_t
mac_frame_set_dst_short_addr(mac_frame_t *frame, short_addr_t short_addr);

ns_error_t
mac_frame_set_dst_ext_addr(mac_frame_t *frame, ext_addr_t *ext_addr);

bool
mac_frame_is_src_panid_present(mac_frame_t *frame, uint16_t fcf);

ns_error_t
mac_frame_get_src_panid(mac_frame_t *frame, panid_t *panid);

ns_error_t
mac_frame_set_src_panid(mac_frame_t *frame, panid_t *panid);

ns_error_t
mac_frame_get_src_addr(mac_frame_t *frame, mac_addr_t *mac_addr);

ns_error_t
mac_frame_set_src_short_addr(mac_frame_t *frame, short_addr_t short_addr);

ns_error_t
mac_frame_set_src_ext_addr(mac_frame_t *frame, ext_addr_t *ext_addr);

ns_error_t
mac_frame_set_src_addr(mac_frame_t *frame, mac_addr_t *macaddr);

ns_error_t
mac_frame_get_security_level(mac_frame_t *frame, uint8_t *security_level);

ns_error_t
mac_frame_get_key_id_mode(mac_frame_t *frame, uint8_t *key_id_mode);

ns_error_t
mac_frame_get_frame_counter(mac_frame_t *frame, uint32_t *frame_counter);

ns_error_t
mac_frame_set_frame_counter(mac_frame_t *frame, uint32_t frame_counter);

const uint8_t *
mac_frame_get_key_source(mac_frame_t *frame);

void
mac_frame_set_key_source(mac_frame_t *frame, const uint8_t *key_source);

ns_error_t
mac_frame_get_key_id(mac_frame_t *frame, uint8_t *key_id);

ns_error_t
mac_frame_set_key_id(mac_frame_t *frame, uint8_t key_id);

ns_error_t
mac_frame_get_command_id(mac_frame_t *frame, uint8_t command_id);

ns_error_t
mac_frame_set_command_id(mac_frame_t *frame, uint8_t *command_id);

bool
mac_frame_is_data_request_command(mac_frame_t *frame);

uint8_t
mac_frame_get_length(mac_frame_t *frame);

ns_error_t
mac_frame_set_length(mac_frame_t *frame, uint8_t length);

uint8_t
mac_frame_get_header_length(mac_frame_t *frame);

uint8_t
mac_frame_get_footer_length(mac_frame_t *frame);

uint8_t
mac_frame_get_payload_length(mac_frame_t *frame);

uint8_t
mac_frame_get_max_payload_length(mac_frame_t *frame);

ns_error_t
mac_frame_set_payload_length(mac_frame_t *frame, uint8_t length);

uint8_t
mac_frame_get_channel(mac_frame_t *frame);

void
mac_frame_set_channel(mac_frame_t *frame, uint8_t channel);

int8_t
mac_frame_get_rssi(mac_frame_t *frame);

void
mac_frame_set_rssi(mac_frame_t *frame, int8_t rssi);

uint8_t
mac_frame_get_lqi(mac_frame_t *frame);

void
mac_frame_set_lqi(mac_frame_t *frame, uint8_t lqi);

uint8_t
mac_frame_get_max_csma_backoffs(mac_frame_t *frame);

void
mac_frame_set_max_csma_backoffs(mac_frame_t *frame, uint8_t max_csma_backoffs);

uint8_t
mac_frame_max_frame_retries(mac_frame_t *frame);

void
mac_frame_set_max_frame_retries(mac_frame_t *frame, uint8_t max_frame_retries);

bool
mac_frame_is_a_retransmission(mac_frame_t *frame);

void
mac_frame_set_is_a_retransmission(mac_frame_t *frame, bool is_a_retx);

void
mac_frame_set_did_tx(mac_frame_t *frame, bool did_tx);

void
mac_frame_set_csma_ca_enabled(mac_frame_t *frame, bool csma_ca_enabled);

bool
mac_frame_is_csma_ca_enabled(mac_frame_t *frame);

const uint8_t *
mac_frame_get_aes_key(mac_frame_t *frame);

void
mac_frame_set_aes_key(const uint8_t *aes_key);

uint8_t
mac_frame_get_psdu_length(mac_frame_t *frame);

void
mac_frame_set_psdu_length(uint8_t length);

uint8_t *
mac_frame_get_psdu(mac_frame_t *frame);

uint8_t *
mac_frame_get_header(mac_frame_t *frame);

uint8_t *
mac_frame_get_payload(mac_frame_t *frame);

uint8_t *
mac_frame_get_footer(mac_frame_t *frame);

#if NS_CONFIG_ENABLE_TIME_SYNC
void
mac_frame_set_time_ie_offset(mac_frame_t *frame, uint8_t offset);

void
mac_frame_set_network_time_offset(mac_frame_t *frame, int64_t network_time_offset);

int64_t
mac_frame_get_network_time_offset(mac_frame_t *frame);

void
mac_frame_set_time_sync_seq(mac_frame_t *frame, uint8_t time_sync_seq);

uint8_t
mac_frame_get_time_sync_seq(mac_frame_t *frame);

uint64_t
mac_frame_get_time_stamp(mac_frame_t *frame);

uint8_t
mac_frame_get_time_ie(mac_frame_t *frame);
#endif // NS_CONFIG_ENABLE_TIME_SYNC

#if NS_CONFIG_HEADER_IE_SUPPORT
ns_error_t
mac_frame_append_header_ie(mac_frame_t *frame, header_ie_t *header_ie, uint8_t ie_count);

uint8_t *
mac_frame_get_header_ie(mac_frame_t *frame, uint8_t ie_id);
#endif // NS_CONFIG_HEADER_IE_SUPPORT

// --- MAC beacon functions
void
mac_beacon_init(mac_beacon_t *beacon);

bool
mac_beacon_is_valid(mac_beacon_t *beacon);

uint8_t *
mac_beacon_get_payload(mac_beacon_t *beacon);

// --- MAC beacon payload functions
void
mac_beacon_payload_init(mac_beacon_payload_t *beacon_payload);

bool
mac_beacon_payload_is_valid(mac_beacon_payload_t *beacon_payload);

uint8_t
mac_beacon_payload_get_protocol_id(mac_beacon_payload_t *beacon_payload);

uint8_t
mac_beacon_payload_get_protocol_version(mac_beacon_payload_t *beacon_payload);

bool
mac_beacon_payload_is_native(mac_beacon_payload_t *beacon_payload);

void
mac_beacon_payload_clear_native(mac_beacon_payload_t *beacon_payload);

void
mac_beacon_payload_set_native(mac_beacon_payload_t *beacon_payload);

bool
mac_beacon_payload_is_joining_permitted(mac_beacon_payload_t *beacon_payload);

void
mac_beacon_payload_clear_joining_permitted(mac_beacon_payload_t *beacon_payload);

void
mac_beacon_payload_set_joining_permitted(mac_beacon_payload_t *beacon_payload);

const char *
mac_beacon_payload_get_network_name(mac_beacon_payload_t *beacon_payload);

void
mac_beacon_payload_set_network_name(mac_beacon_payload_t *beacon_payload, const char *network_name);

const uint8_t *
mac_beacon_payload_get_extended_panid(mac_beacon_payload_t *beacon_payload);

void
mac_beacon_payload_set_extended_panid(mac_beacon_payload_t *beacon_payload, const uint8_t *ext_panid);

#if NS_CONFIG_ENABLE_TIME_SYNC
// --- MAC vendor ie header functions
const uint8_t *
mac_vendor_ie_header_get_vendor_oui(mac_vendor_ie_header_t *vendor_ie_header);

void
mac_vendor_ie_header_set_vendor_oui(mac_vendor_ie_header_t *vendor_ie_header, uint8_t *vendor_oui);

uint8_t
mac_vendor_ie_header_get_sub_type(mac_vendor_ie_header_t *vendor_ie_header);

void
mac_vendor_ie_header_set_sub_type(mac_vendor_ie_header_t *vendor_ie_header, uint8_t sub_type);

// --- MAC time ie funtions
void
mac_time_ie_init(mac_time_ie_t *mac_time_ie);

uint8_t
mac_time_ie_get_sequence(mac_time_ie_t *mac_time_ie);

void
mac_time_ie_set_sequence(mac_time_ie_t *mac_time_ie, uint8_t sequence);

uint64_t
mac_time_ie_get_time(mac_time_ie_t *mac_time_ie);

void
mac_time_ie_set_time(mac_time_ie_t *mac_time_ie, uint64_t time);
#endif // NS_CONFIG_ENABLE_TIME_SYNC


