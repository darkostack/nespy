#ifndef NS_CORE_MAC_MAC_FRAME_H_
#define NS_CORE_MAC_MAC_FRAME_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/platform/radio.h"
#include "ns/sys/core/common/string.h"
#include <limits.h>

typedef ns_panid_t panid_t;
typedef ns_short_addr_t short_addr_t;
typedef ns_ext_addr_t ext_addr_t;
typedef struct _mac_addr mac_addr_t;
typedef ns_radio_frame_t mac_frame_t;

enum {
    MAC_SHORT_ADDR_BROADCAST = 0xffff,
    MAC_SHORT_ADDR_INVALID = 0xfffe,
    MAC_PANID_BROADCAST = 0xffff,
};

enum {
    MAC_EXT_ADDR_GROUP_FLAG = 1 << 0,
    MAC_EXT_ADDR_LOCAL_FLAG = 1 << 1,
};

enum {
    MAC_EXT_ADDR_INFO_STRING_SIZE = 17,
};

typedef enum _mac_addr_type {
    MAC_ADDR_TYPE_NONE,
    MAC_ADDR_TYPE_SHORT,
    MAC_ADDR_TYPE_EXTENDED,
} mac_addr_type_t;

enum {
    MAC_HEADER_IE_ID_OFFSET = 7,
    MAC_HEADER_IE_MASK = 0xff << MAC_HEADER_IE_ID_OFFSET,
    MAC_HEADER_IE_LENGTH_OFFSET = 0,
    MAC_HEADER_IE_LENGTH_MASK = 0x7f << MAC_HEADER_IE_LENGTH_OFFSET,
};

typedef uint16_t header_ie_t;

struct _mac_addr {
    union {
        short_addr_t short_addr;
        ext_addr_t ext_addr;
    } shared;
    mac_addr_type_t type;
};

enum {
    MAC_FRAME_MTU                   = 127,
    MAC_FRAME_FCF_SIZE              = sizeof(uint16_t),
    MAC_FRAME_DSN_SIZE              = sizeof(uint8_t),
    MAC_FRAME_SECURITY_CONTROL_SIZE = sizeof(uint8_t),
    MAC_FRAME_FRAME_COUNTER_SIZE    = sizeof(uint32_t),
    MAC_FRAME_COMMAND_ID_SIZE       = sizeof(uint8_t),
    MAC_FRAME_FCS_SIZE              = sizeof(uint16_t),

    MAC_FRAME_FCF_FRAME_BEACON           = 0 << 0,
    MAC_FRAME_FCF_FRAME_DATA             = 1 << 0,
    MAC_FRAME_FCF_FRAME_ACK              = 2 << 0,
    MAC_FRAME_FCF_FRAME_MAC_CMD          = 3 << 0,
    MAC_FRAME_FCF_FRAME_TYPE_MASK        = 7 << 0,
    MAC_FRAME_FCF_FRAME_SECURITY_ENABLED = 1 << 3,
    MAC_FRAME_FCF_FRAME_PENDING          = 1 << 4,
    MAC_FRAME_FCF_ACK_REQUEST            = 1 << 5,
    MAC_FRAME_FCF_PANID_COMPRESSION      = 1 << 6,
    MAC_FRAME_FCF_IE_PRESENT             = 1 << 9,
    MAC_FRAME_FCF_DST_ADDR_NONE          = 0 << 10,
    MAC_FRAME_FCF_DST_ADDR_SHORT         = 2 << 10,
    MAC_FRAME_FCF_DST_ADDR_EXT           = 3 << 10,
    MAC_FRAME_FCF_DST_ADDR_MASK          = 3 << 10,
    MAC_FRAME_FCF_FRAME_VERSION_2006     = 1 << 12,
    MAC_FRAME_FCF_FRAME_VERSION_2015     = 2 << 12,
    MAC_FRAME_FCF_FRAME_VERSION_MASK     = 3 << 12,
    MAC_FRAME_FCF_SRC_ADDR_NONE          = 0 << 14,
    MAC_FRAME_FCF_SRC_ADDR_SHORT         = 2 << 14,
    MAC_FRAME_FCF_SRC_ADDR_EXT           = 3 << 14,
    MAC_FRAME_FCF_SRC_ADDR_MASK          = 3 << 14,

    MAC_FRAME_SEC_NONE        = 0 << 0,
    MAC_FRAME_SEC_MIC_32      = 1 << 0,
    MAC_FRAME_SEC_MIC_64      = 2 << 0,
    MAC_FRAME_SEC_MIC_128     = 3 << 0,
    MAC_FRAME_SEC_ENC         = 4 << 0,
    MAC_FRAME_SEC_ENC_MIC_32  = 5 << 0,
    MAC_FRAME_SEC_ENC_MIC_64  = 6 << 0,
    MAC_FRAME_SEC_ENC_MIC_128 = 7 << 0,
    MAC_FRAME_SEC_LEVEL_MASK  = 7 << 0,

    MAC_FRAME_MIC_0_SIZE   = 0,
    MAC_FRAME_MIC_32_SIZE  = 32 / CHAR_BIT,
    MAC_FRAME_MIC_64_SIZE  = 64 / CHAR_BIT,
    MAC_FRAME_MIC_128_SIZE = 128 / CHAR_BIT,
    MAC_FRAME_MIC_SIZE     = MAC_FRAME_MIC_128_SIZE,

    MAC_FRAME_KEY_ID_MODE_0    = 0 << 3,
    MAC_FRAME_KEY_ID_MODE_1    = 1 << 3,
    MAC_FRAME_KEY_ID_MODE_2    = 2 << 3,
    MAC_FRAME_KEY_ID_MODE_3    = 3 << 3,
    MAC_FRAME_KEY_ID_MODE_MASK = 3 << 3,

    MAC_FRAME_KEY_SOURCE_SIZE_MODE_0 = 0,
    MAC_FRAME_KEY_SOURCE_SIZE_MODE_1 = 0,
    MAC_FRAME_KEY_SOURCE_SIZE_MODE_2 = 4,
    MAC_FRAME_KEY_SOURCE_SIZE_MODE_3 = 8,

    MAC_FRAME_KEY_INDEX_SIZE = sizeof(uint8_t),

    MAC_FRAME_MAC_CMD_ASSOCIATION_REQUEST         = 1,
    MAC_FRAME_MAC_CMD_ASSOCIATION_RESPONSE        = 2,
    MAC_FRAME_MAC_CMD_DISASSOCIATION_NOTIFICATION = 3,
    MAC_FRAME_MAC_CMD_DATA_REQUEST                = 4,
    MAC_FRAME_MAC_CMD_PANID_CONFLICT_NOTIFICATION = 5,
    MAC_FRAME_MAC_CMD_ORPHAN_NOTIFICATION         = 6,
    MAC_FRAME_MAC_CMD_BEACON_REQUEST              = 7,
    MAC_FRAME_MAC_CMD_COORDINATOR_REALIGNMENT     = 8,
    MAC_FRAME_MAC_CMD_GTS_REQUEST                 = 9,

    MAC_FRAME_HEADER_IE_VENDOR        = 0x00,
    MAC_FRAME_HEADER_IE_TERMINATION_2 = 0x7f,
    MAC_FRAME_VENDOR_OUI_NEST         = 0x18b430,
    MAC_FRAME_VENDOR_OUI_SIZE         = 3,
    MAC_FRAME_VENDOR_IE_TIME          = 0x01,

    MAC_FRAME_INFO_STRING_SIZE = 110,
};

enum {
    MAC_FRAME_INVALID_INDEX = 0xff,
    MAC_FRAME_SEQUENCE_INDEX = MAC_FRAME_FCF_SIZE,
};

enum {
    MAC_BEACON_SUPER_FRAME_SPEC = 0x0fff,
};

enum {
    MAC_BEACON_PAYLOAD_PROTOCOL_ID = 3,
    MAC_BEACON_PAYLOAD_NETWORK_NAME_SIZE = 16,
    MAC_BEACON_PAYLOAD_EXT_PANID_SIZE = 8,
    MAC_BEACON_INFO_STRING_SIZE = 92,
};

enum {
    MAC_BEACON_PAYLOAD_PROTOCOL_VERSION = 2,
    MAC_BEACON_PAYLOAD_VERSION_OFFSET = 4,
    MAC_BEACON_PAYLOAD_VERSION_MASK = 0xf << MAC_BEACON_PAYLOAD_VERSION_OFFSET,
    MAC_BEACON_PAYLOAD_NATIVE_FLAG = 1 << 3,
    MAC_BEACON_PAYLOAD_JOINING_FLAG = 1 << 0,
};

typedef struct _mac_beacon {
    uint16_t super_frame_spec;
    uint8_t gts_spec;
    uint8_t pending_addr_spec;
} mac_beacon_t;

typedef struct _mac_beacon_payload {
    uint8_t protocol_id;
    uint8_t flags;
    char network_name[MAC_BEACON_PAYLOAD_NETWORK_NAME_SIZE];
    uint8_t extended_panid[MAC_BEACON_PAYLOAD_EXT_PANID_SIZE];
} mac_beacon_payload_t;

#if NS_CONFIG_ENABLE_TIME_SYNC
typedef struct _mac_vendor_ie_header {
    uint8_t vendor_oui[MAC_FRAME_VENDOR_OUI_SIZE];
    uint8_t sub_type;
} mac_vendor_ie_header_t;

typedef struct _mac_time_ie {
    mac_vendor_ie_header_t vendor_ie_header;
    uint8_t sequence;
    uint64_t time;
} mac_time_ie_t;
#endif // NS_CONFIG_ENABLE_TIME_SYNC

// --- MAC extended address functions
void
mac_ext_addr_gen_random(ext_addr_t *ext_addr);

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

string_t *
mac_ext_addr_to_string(ext_addr_t *ext_addr);

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

ext_addr_t *
mac_addr_get_extended(mac_addr_t *mac_addr);

void
mac_addr_set_none(mac_addr_t *mac_addr);

void
mac_addr_set_short(mac_addr_t *mac_addr, short_addr_t short_addr);

void
mac_addr_set_extended(mac_addr_t *mac_addr, ext_addr_t ext_addr);

void
mac_addr_set_extended_from_buf(mac_addr_t *mac_addr, const uint8_t *buf, bool reverse);

bool
mac_addr_is_broadcast(mac_addr_t *mac_addr);

bool
mac_addr_is_short_addr_invalid(mac_addr_t *mac_addr);

string_t *
mac_addr_to_string(mac_addr_t *mac_addr);

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

#endif // NS_CORE_MAC_MAC_FRAME_H_
