#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/random.h"
#include "ns/sys/core/common/encoding.h"
#include "ns/sys/core/mac/mac_frame.h"
#include <string.h>

STRING(mac_ext_addr_info, MAC_EXT_ADDR_INFO_STRING_SIZE);
STRING(mac_addr_info, sizeof(mac_ext_addr_info_string));

// --- MAC frame private functions declarations
uint16_t
frame_get_frame_control_field(mac_frame_t *frame);

uint8_t
frame_find_dst_panid_index(mac_frame_t *frame);

uint8_t
frame_find_dst_addr_index(mac_frame_t *frame);

uint8_t
frame_find_src_panid_index(mac_frame_t *frame);

uint8_t
frame_find_src_addr_index(mac_frame_t *frame);

uint8_t
frame_find_security_header_index(mac_frame_t *frame);

uint8_t
frame_skip_security_header_index(mac_frame_t *frame);

uint8_t
frame_find_payload_index(mac_frame_t *frame);

#if NS_CONFIG_HEADER_IE_SUPPORT
uint8_t
frame_find_header_ie_index(mac_frame_t *frame);
#endif // NS_CONFIG_HEADER_IE_SUPPORT

// --- MAC extended address functions
void
mac_ext_addr_gen_random(ext_addr_t *ext_addr)
{
    random_fill_buffer(ext_addr->m8, sizeof(ext_addr_t));
    mac_ext_addr_set_group(ext_addr, false);
    mac_ext_addr_set_local(ext_addr, true);
}

bool
mac_ext_addr_is_group(ext_addr_t *ext_addr)
{
    return ((ext_addr->m8[0] & MAC_EXT_ADDR_GROUP_FLAG) != 0);
}

void
mac_ext_addr_set_group(ext_addr_t *ext_addr, bool group)
{
    if (group) {
        ext_addr->m8[0] |= MAC_EXT_ADDR_GROUP_FLAG;
    } else {
        ext_addr->m8[0] &= ~MAC_EXT_ADDR_GROUP_FLAG;
    }
}

void
mac_ext_addr_toggle_group(ext_addr_t *ext_addr)
{
    ext_addr->m8[0] ^= MAC_EXT_ADDR_GROUP_FLAG;
}

bool
mac_ext_addr_is_local(ext_addr_t *ext_addr)
{
    return ((ext_addr->m8[0] & MAC_EXT_ADDR_LOCAL_FLAG) != 0);
}

void
mac_ext_addr_set_local(ext_addr_t *ext_addr, bool local)
{
    if (local) {
        ext_addr->m8[0] |= MAC_EXT_ADDR_LOCAL_FLAG;
    } else {
        ext_addr->m8[0] &= ~MAC_EXT_ADDR_LOCAL_FLAG;
    }
}

void
mac_ext_addr_toggle_local(ext_addr_t *ext_addr)
{
    ext_addr->m8[0] ^= MAC_EXT_ADDR_LOCAL_FLAG;
}

bool
mac_ext_addr_is_equal(ext_addr_t *ext_addr1, ext_addr_t *ext_addr2)
{
    return (memcmp(ext_addr1->m8, ext_addr2->m8, sizeof(ext_addr_t)) == 0);
}

string_t *
mac_ext_addr_to_string(ext_addr_t *ext_addr)
{
    string_set(&mac_ext_addr_info_string, "%02x%02x%02x%02x%02x%02x%02x%02x",
               ext_addr->m8[0], ext_addr->m8[1], ext_addr->m8[2], ext_addr->m8[3],
               ext_addr->m8[4], ext_addr->m8[5], ext_addr->m8[6], ext_addr->m8[7]);
    return &mac_ext_addr_info_string;
}

// --- MAC address functions
void
mac_addr_make_new(mac_addr_t *mac_addr)
{
    mac_addr->type = MAC_ADDR_TYPE_NONE;
}

mac_addr_type_t
mac_addr_get_type(mac_addr_t *mac_addr)
{
    return mac_addr->type;
}

bool
mac_addr_type_is_none(mac_addr_t *mac_addr)
{
    return (mac_addr->type == MAC_ADDR_TYPE_NONE);
}

bool
mac_addr_type_is_short(mac_addr_t *mac_addr)
{
    return (mac_addr->type == MAC_ADDR_TYPE_SHORT);
}

bool
mac_addr_type_is_extended(mac_addr_t *mac_addr)
{
    return (mac_addr->type == MAC_ADDR_TYPE_EXTENDED);
}

short_addr_t
mac_addr_get_short(mac_addr_t *mac_addr)
{
    return mac_addr->shared.short_addr;
}

ext_addr_t *
mac_addr_get_extended(mac_addr_t *mac_addr)
{
    return &mac_addr->shared.ext_addr;
}

void
mac_addr_set_none(mac_addr_t *mac_addr)
{
    mac_addr->type = MAC_ADDR_TYPE_NONE;
}

void
mac_addr_set_short(mac_addr_t *mac_addr, short_addr_t short_addr)
{
    mac_addr->shared.short_addr = short_addr;
    mac_addr->type = MAC_ADDR_TYPE_SHORT;
}

void
mac_addr_set_extended(mac_addr_t *mac_addr, ext_addr_t ext_addr)
{
    mac_addr->shared.ext_addr = ext_addr;
    mac_addr->type = MAC_ADDR_TYPE_EXTENDED;
}

void
mac_addr_set_extended_from_buffer(mac_addr_t *mac_addr, const uint8_t *buffer, bool reverse)
{
    mac_addr->type = MAC_ADDR_TYPE_EXTENDED;
    if (reverse) {
        for (unsigned int i = 0; i < sizeof(ext_addr_t); i++) {
            mac_addr->shared.ext_addr.m8[i] = buffer[sizeof(ext_addr_t) - 1 - i];
        }
    } else {
        memcpy(mac_addr->shared.ext_addr.m8, buffer, sizeof(ext_addr_t));
    }
}

bool
mac_addr_is_broadcast(mac_addr_t *mac_addr)
{
    return ((mac_addr->type == MAC_ADDR_TYPE_SHORT) &&
            (mac_addr_get_short(mac_addr) == MAC_SHORT_ADDR_BROADCAST));
}

bool
mac_addr_is_short_addr_invalid(mac_addr_t *mac_addr)
{
    return ((mac_addr->type == MAC_ADDR_TYPE_SHORT) &&
            (mac_addr_get_short(mac_addr) == MAC_SHORT_ADDR_INVALID));
}

string_t *
mac_addr_to_string(mac_addr_t *mac_addr)
{
    string_clear(&mac_addr_info_string);
    if (mac_addr->type == MAC_ADDR_TYPE_EXTENDED) {
        string_t *ext_addr_string = mac_ext_addr_to_string(mac_addr_get_extended(mac_addr));
        string_copy(&mac_addr_info_string,
                    ext_addr_string,
                    string_get_length(ext_addr_string));
    } else {
        if (mac_addr->type == MAC_ADDR_TYPE_NONE) {
            string_set(&mac_addr_info_string, "None");
        } else {
            string_set(&mac_addr_info_string, "0x%04x", mac_addr_get_short(mac_addr));
        }
    }
    return &mac_addr_info_string;
}

// --- MAC header ie functions
void
mac_header_ie_init(header_ie_t *header_ie)
{
    header_ie->ie = 0;
}

uint16_t
mac_header_ie_get_id(header_ie_t *header_ie)
{
    return (encoding_little_endian_swap16(header_ie->ie) & MAC_HEADER_IE_ID_MASK) >> MAC_HEADER_IE_ID_OFFSET;
}

void
mac_header_ie_set_id(header_ie_t *header_ie, uint16_t id)
{
    header_ie->ie = encoding_little_endian_swap16(
            (encoding_little_endian_swap16(header_ie->ie) & ~MAC_HEADER_IE_ID_MASK) |
            ((id << MAC_HEADER_IE_ID_OFFSET) & MAC_HEADER_IE_ID_MASK));
}

uint16_t
mac_header_ie_get_length(header_ie_t *header_ie)
{
    return (encoding_little_endian_swap16(header_ie->ie) & MAC_HEADER_IE_LENGTH_MASK) >> MAC_HEADER_IE_LENGTH_OFFSET;
}

void
mac_header_ie_set_length(header_ie_t *header_ie, uint16_t length)
{
    header_ie->ie = encoding_little_endian_swap16(
            (encoding_little_endian_swap16(header_ie->ie) & ~MAC_HEADER_IE_LENGTH_MASK) |
            ((length << MAC_HEADER_IE_LENGTH_OFFSET) & MAC_HEADER_IE_LENGTH_MASK));
}

// --- MAC frame functions
ns_error_t
mac_frame_init_mac_header(mac_frame_t *frame, uint16_t fcf, uint8_t sec_ctl)
{
    uint8_t *bytes = mac_frame_get_psdu(frame);
    uint8_t length = 0;

    // frame control field
    encoding_little_endian_write_uint16(fcf, bytes);
    length += MAC_FRAME_FCF_SIZE;

    // sequence number
    length += MAC_FRAME_DSN_SIZE;

    // destination PAN + address
    switch (fcf & MAC_FRAME_FCF_DST_ADDR_MASK) {
    case MAC_FRAME_FCF_DST_ADDR_NONE:
        break;
    case MAC_FRAME_FCF_DST_ADDR_SHORT:
        length += sizeof(panid_t) + sizeof(short_addr_t);
        break;
    case MAC_FRAME_FCF_DST_ADDR_EXT:
        length += sizeof(panid_t) + sizeof(ext_addr_t);
        break;
    default:
        ns_assert(false);
    }

    // source PAN
    if (mac_frame_is_src_panid_present(frame, fcf)) {
        length += sizeof(panid_t);
    }

    // source address
    switch (fcf & MAC_FRAME_FCF_SRC_ADDR_MASK) {
    case MAC_FRAME_FCF_SRC_ADDR_NONE:
        break;
    case MAC_FRAME_FCF_SRC_ADDR_SHORT:
        length += sizeof(short_addr_t);
        break;
    case MAC_FRAME_FCF_SRC_ADDR_EXT:
        length += sizeof(ext_addr_t);
        break;
    default:
        ns_assert(false);
    }

    // security header
    if (fcf & MAC_FRAME_FCF_SECURITY_ENABLED) {
        bytes[length] = sec_ctl;
        if (sec_ctl & MAC_FRAME_SEC_LEVEL_MASK) {
            length += MAC_FRAME_SECURITY_CONTROL_SIZE + MAC_FRAME_FRAME_COUNTER_SIZE;
        }
        switch (sec_ctl & MAC_FRAME_KEY_ID_MODE_MASK) {
        case MAC_FRAME_KEY_ID_MODE_0:
            length += MAC_FRAME_KEY_SOURCE_SIZE_MODE_0;
            break;
        case MAC_FRAME_KEY_ID_MODE_1:
            length += MAC_FRAME_KEY_SOURCE_SIZE_MODE_1 + MAC_FRAME_KEY_INDEX_SIZE;
            break;
        case MAC_FRAME_KEY_ID_MODE_2:
            length += MAC_FRAME_KEY_SOURCE_SIZE_MODE_2 + MAC_FRAME_KEY_INDEX_SIZE;
            break;
        case MAC_FRAME_KEY_ID_MODE_3:
            length += MAC_FRAME_KEY_SOURCE_SIZE_MODE_3 + MAC_FRAME_KEY_INDEX_SIZE;
            break;
        }
    }

    // command id
    if ((fcf & MAC_FRAME_FCF_FRAME_TYPE_MASK) == MAC_FRAME_FCF_FRAME_MAC_CMD) {
        length += MAC_FRAME_COMMAND_ID_SIZE;
    }

    mac_frame_set_psdu_length(frame, length + mac_frame_get_footer_length(frame));

    return NS_ERROR_NONE;
}

ns_error_t
mac_frame_validate_psdu(mac_frame_t *frame)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t offset = frame_find_payload_index(frame);
    VERIFY_OR_EXIT(offset != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    VERIFY_OR_EXIT((offset + mac_frame_get_footer_length(frame)) <= mac_frame_get_psdu_length(frame),
                   error = NS_ERROR_PARSE);
exit:
    return error;
}

uint8_t
mac_frame_get_type(mac_frame_t *frame)
{
    return mac_frame_get_psdu(frame)[0] & MAC_FRAME_FCF_FRAME_TYPE_MASK;
}

uint16_t
mac_frame_get_version(mac_frame_t *frame)
{
    return frame_get_frame_control_field(frame) & MAC_FRAME_FCF_FRAME_VERSION_MASK;
}

bool
mac_frame_get_security_enabled(mac_frame_t *frame)
{
    return mac_frame_get_psdu(frame)[0] & MAC_FRAME_FCF_SECURITY_ENABLED;
}

bool
mac_frame_get_frame_pending(mac_frame_t *frame)
{
    return ((mac_frame_get_psdu(frame)[0] & MAC_FRAME_FCF_FRAME_PENDING) != 0);
}

void
mac_frame_set_frame_pending(mac_frame_t *frame, bool frame_pending)
{
    if (frame_pending) {
        mac_frame_get_psdu(frame)[0] |= MAC_FRAME_FCF_FRAME_PENDING;
    } else {
        mac_frame_get_psdu(frame)[0] &= ~MAC_FRAME_FCF_FRAME_PENDING;
    }
}

bool
mac_frame_get_ack_request(mac_frame_t *frame)
{
    return ((mac_frame_get_psdu(frame)[0] & MAC_FRAME_FCF_ACK_REQUEST) != 0);
}

void
mac_frame_set_ack_request(mac_frame_t *frame, bool ack_request)
{
    if (ack_request) {
        mac_frame_get_psdu(frame)[0] |= MAC_FRAME_FCF_ACK_REQUEST;
    } else {
        mac_frame_get_psdu(frame)[0] &= ~MAC_FRAME_FCF_ACK_REQUEST;
    }
}

bool
mac_frame_is_ie_present(mac_frame_t *frame)
{
    return ((frame_get_frame_control_field(frame) & MAC_FRAME_FCF_IE_PRESENT) != 0);
}

uint8_t
mac_frame_get_sequence(mac_frame_t *frame)
{
    return mac_frame_get_psdu(frame)[MAC_FRAME_SEQUENCE_INDEX];
}

void
mac_frame_set_sequence(mac_frame_t *frame, uint8_t sequence)
{
    mac_frame_get_psdu(frame)[MAC_FRAME_SEQUENCE_INDEX] = sequence;
}

ns_error_t
mac_frame_get_dst_panid(mac_frame_t *frame, panid_t *panid)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_dst_panid_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    *panid = encoding_little_endian_read_uint16(mac_frame_get_psdu(frame) + index);
exit:
    return error;
}

ns_error_t
mac_frame_set_dst_panid(mac_frame_t *frame, panid_t panid)
{
    uint8_t index = frame_find_dst_panid_index(frame);
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    encoding_little_endian_write_uint16(panid, mac_frame_get_psdu(frame) + index);
    return NS_ERROR_NONE;
}

ns_error_t
mac_frame_get_dst_addr(mac_frame_t *frame, mac_addr_t *mac_addr)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_dst_addr_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    switch (frame_get_frame_control_field(frame) & MAC_FRAME_FCF_DST_ADDR_MASK) {
    case MAC_FRAME_FCF_DST_ADDR_SHORT:
        mac_addr_set_short(mac_addr, encoding_little_endian_read_uint16(mac_frame_get_psdu(frame) + index));
        break;
    case MAC_FRAME_FCF_DST_ADDR_EXT:
        mac_addr_set_extended_from_buffer(mac_addr, mac_frame_get_psdu(frame) + index, /* reverse */ true);
        break;
    default:
        mac_addr_set_none(mac_addr);
        break;
    }
exit:
    return error;
}

ns_error_t
mac_frame_set_dst_addr_short(mac_frame_t *frame, short_addr_t short_addr)
{
    ns_assert((frame_get_frame_control_field(frame) & MAC_FRAME_FCF_DST_ADDR_MASK) == MAC_FRAME_FCF_DST_ADDR_SHORT);
    encoding_little_endian_write_uint16(short_addr, mac_frame_get_psdu(frame) + frame_find_dst_addr_index(frame));
    return NS_ERROR_NONE;
}

ns_error_t
mac_frame_set_dst_addr_ext(mac_frame_t *frame, ext_addr_t *ext_addr)
{
    uint8_t index = frame_find_dst_addr_index(frame);
    uint8_t *buf = mac_frame_get_psdu(frame) + index;
    ns_assert((frame_get_frame_control_field(frame) & MAC_FRAME_FCF_DST_ADDR_MASK) == MAC_FRAME_FCF_DST_ADDR_EXT);
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    for (unsigned int i = 0; i < sizeof(ext_addr_t); i++) {
        buf[i] = ext_addr->m8[sizeof(ext_addr_t) - 1 - i];
    }
    return NS_ERROR_NONE;
}

bool
mac_frame_is_src_panid_present(mac_frame_t *frame, uint16_t fcf)
{
    bool src_panid_present = false;
    if ((fcf & MAC_FRAME_FCF_SRC_ADDR_MASK) != MAC_FRAME_FCF_SRC_ADDR_NONE &&
        (fcf & MAC_FRAME_FCF_PANID_COMPRESSION) == 0) {
#if NS_CONFIG_HEADER_IE_SUPPORT
        // Handle a special case in IEEE 802.15.4-2015, when Pan ID Compression is 0,
        // but Src Pan ID is not present:

        //  Dest Address:       Extended
        //  Source Address:     Extended
        //  Dest Pan ID:        Present
        //  Src Pan ID:         Not Present
        //  Pan ID Compression: 0

        if ((fcf & MAC_FRAME_FCF_FRAME_VERSION_MASK) != MAC_FRAME_FCF_FRAME_VERSION_2015 ||
            (fcf & MAC_FRAME_FCF_DST_ADDR_MASK) != MAC_FRAME_FCF_DST_ADDR_EXT ||
            (fcf & MAC_FRAME_FCF_SRC_ADDR_MASK) != MAC_FRAME_FCF_SRC_ADDR_EXT)
#endif // NS_CONFIG_HEADER_IE_SUPPORT
        {
            src_panid_present = true;
        }
    }

    return src_panid_present;
}

ns_error_t
mac_frame_get_src_panid(mac_frame_t *frame, panid_t *panid)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_src_panid_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    *panid = encoding_little_endian_read_uint16(mac_frame_get_psdu(frame) + index);
exit:
    return error;
}

ns_error_t
mac_frame_set_src_panid(mac_frame_t *frame, panid_t panid)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_src_panid_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    encoding_little_endian_write_uint16(panid, mac_frame_get_psdu(frame) + index);
exit:
    return error;
}

ns_error_t
mac_frame_get_src_addr(mac_frame_t *frame, mac_addr_t *mac_addr)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_src_addr_index(frame);
    uint16_t fcf = frame_get_frame_control_field(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    switch (fcf & MAC_FRAME_FCF_SRC_ADDR_MASK) {
    case MAC_FRAME_FCF_SRC_ADDR_SHORT:
        mac_addr_set_short(mac_addr, encoding_little_endian_read_uint16(mac_frame_get_psdu(frame) + index));
        break;
    case MAC_FRAME_FCF_SRC_ADDR_EXT:
        mac_addr_set_extended_from_buffer(mac_addr, mac_frame_get_psdu(frame) + index, /* reverse */ true);
        break;
    default:
        mac_addr_set_none(mac_addr);
        break;
    }
exit:
    return error;
}

ns_error_t
mac_frame_set_src_addr_short(mac_frame_t *frame, short_addr_t short_addr)
{
    uint8_t index = frame_find_src_addr_index(frame);
    ns_assert((frame_get_frame_control_field(frame) & MAC_FRAME_FCF_SRC_ADDR_MASK) == MAC_FRAME_FCF_SRC_ADDR_SHORT);
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    encoding_little_endian_write_uint16(short_addr, mac_frame_get_psdu(frame) + index);
    return NS_ERROR_NONE;
}

ns_error_t
mac_frame_set_src_addr_ext(mac_frame_t *frame, ext_addr_t *ext_addr)
{
    uint8_t index = frame_find_src_addr_index(frame);
    uint8_t *buf = mac_frame_get_psdu(frame) + index;
    ns_assert((frame_get_frame_control_field(frame) & MAC_FRAME_FCF_SRC_ADDR_MASK) == MAC_FRAME_FCF_SRC_ADDR_EXT);
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    for (unsigned int i = 0; i < sizeof(ext_addr_t); i++) {
        buf[i] = ext_addr->m8[sizeof(ext_addr_t) - 1 - i];
    }
    return NS_ERROR_NONE;
}

ns_error_t
mac_frame_set_src_addr(mac_frame_t *frame, mac_addr_t *mac_addr)
{
    ns_error_t error = NS_ERROR_NONE;
    switch (mac_addr_get_type(mac_addr)) {
    case MAC_ADDR_TYPE_SHORT:
        error = mac_frame_set_src_addr_short(frame, mac_addr_get_short(mac_addr));
        break;
    case MAC_ADDR_TYPE_EXTENDED:
        error = mac_frame_set_src_addr_ext(frame, mac_addr_get_extended(mac_addr));
        break;
    default:
        ns_assert(false);
        break;
    }
    return error;
}

ns_error_t
mac_frame_get_security_level(mac_frame_t *frame, uint8_t *security_level)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_security_header_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    *security_level = mac_frame_get_psdu(frame)[index] & MAC_FRAME_SEC_LEVEL_MASK;
exit:
    return error;
}

ns_error_t
mac_frame_get_key_id_mode(mac_frame_t *frame, uint8_t *key_id_mode)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_security_header_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    *key_id_mode = mac_frame_get_psdu(frame)[index] & MAC_FRAME_KEY_ID_MODE_MASK;
exit:
    return error;
}

ns_error_t
mac_frame_get_frame_counter(mac_frame_t *frame, uint32_t *frame_counter)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_security_header_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    // security control
    index += MAC_FRAME_SECURITY_CONTROL_SIZE;
    *frame_counter = encoding_little_endian_read_uint32(mac_frame_get_psdu(frame) + index);
exit:
    return error;
}

ns_error_t
mac_frame_set_frame_counter(mac_frame_t *frame, uint32_t frame_counter)
{
    uint8_t index = frame_find_security_header_index(frame);
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    // security control
    index += MAC_FRAME_SECURITY_CONTROL_SIZE;
    encoding_little_endian_write_uint32(frame_counter, mac_frame_get_psdu(frame) + index);
    return NS_ERROR_NONE;
}

const uint8_t *
mac_frame_get_key_source(mac_frame_t *frame)
{
    uint8_t index = frame_find_security_header_index(frame);
    const uint8_t *buf = mac_frame_get_psdu(frame) + index;
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    // security control
    buf += MAC_FRAME_SECURITY_CONTROL_SIZE + MAC_FRAME_FRAME_COUNTER_SIZE;
    return buf;
}

uint8_t
mac_frame_get_key_source_length(mac_frame_t *frame, uint8_t key_id_mode)
{
    (void)frame; // unused variable
    uint8_t rval = 0;
    switch (key_id_mode) {
    case MAC_FRAME_KEY_ID_MODE_0:
        rval = MAC_FRAME_KEY_SOURCE_SIZE_MODE_0;
        break;
    case MAC_FRAME_KEY_ID_MODE_1:
        rval = MAC_FRAME_KEY_SOURCE_SIZE_MODE_1;
        break;
    case MAC_FRAME_KEY_ID_MODE_2:
        rval = MAC_FRAME_KEY_SOURCE_SIZE_MODE_2;
        break;
    case MAC_FRAME_KEY_ID_MODE_3:
        rval = MAC_FRAME_KEY_SOURCE_SIZE_MODE_3;
        break;
    }
    return rval;
}

void
mac_frame_set_key_source(mac_frame_t *frame, const uint8_t *key_source)
{
    uint8_t key_source_length;
    uint8_t index = frame_find_security_header_index(frame);
    uint8_t *buf = mac_frame_get_psdu(frame) + index;
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    key_source_length = mac_frame_get_key_source_length(frame, buf[0] & MAC_FRAME_KEY_ID_MODE_MASK);
    buf += MAC_FRAME_SECURITY_CONTROL_SIZE + MAC_FRAME_FRAME_COUNTER_SIZE;
    memcpy(buf, key_source, key_source_length);
}

ns_error_t
mac_frame_get_key_id(mac_frame_t *frame, uint8_t *key_id)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t key_source_length;
    uint8_t index = frame_find_security_header_index(frame);
    const uint8_t *buf = mac_frame_get_psdu(frame) + index;
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX);
    key_source_length = mac_frame_get_key_source_length(frame, buf[0] & MAC_FRAME_KEY_ID_MODE_MASK);
    buf += MAC_FRAME_SECURITY_CONTROL_SIZE + MAC_FRAME_FRAME_COUNTER_SIZE + key_source_length;
    *key_id = buf[0];
exit:
    return error;
}

ns_error_t
mac_frame_set_key_id(mac_frame_t *frame, uint8_t key_id)
{
    uint8_t key_source_length;
    uint8_t index = frame_find_security_header_index(frame);
    uint8_t *buf = mac_frame_get_psdu(frame) + index;
    ns_assert(index != MAC_FRAME_INVALID_INDEX);
    key_source_length = mac_frame_get_key_source_length(frame, buf[0] & MAC_FRAME_KEY_ID_MODE_MASK);
    buf += MAC_FRAME_SECURITY_CONTROL_SIZE + MAC_FRAME_FRAME_COUNTER_SIZE + key_source_length;
    buf[0] = key_id;
    return NS_ERROR_NONE;
}

ns_error_t
mac_frame_get_command_id(mac_frame_t *frame, uint8_t *command_id)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_payload_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    *command_id = (mac_frame_get_psdu(frame) + index)[-1];
exit:
    return error;
}

ns_error_t
mac_frame_set_command_id(mac_frame_t *frame, uint8_t command_id)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_payload_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_PARSE);
    (mac_frame_get_psdu(frame) + index)[-1] = command_id;
exit:
    return error;
}

bool
mac_frame_is_data_request_command(mac_frame_t *frame)
{
    bool is_data_request = false;
    uint8_t command_id = 0;
    VERIFY_OR_EXIT(mac_frame_get_type(frame) == MAC_FRAME_FCF_FRAME_MAC_CMD);
    VERIFY_OR_EXIT(mac_frame_get_command_id(frame, &command_id) == NS_ERROR_NONE);
    is_data_request = (command_id == MAC_FRAME_MAC_CMD_DATA_REQUEST);
exit:
    return is_data_request;
}

uint8_t
mac_frame_get_length(mac_frame_t *frame)
{
    return mac_frame_get_psdu_length(frame);
}

ns_error_t
mac_frame_set_length(mac_frame_t *frame, uint8_t length)
{
    mac_frame_set_psdu_length(frame, length);
    return NS_ERROR_NONE;
}

uint8_t
mac_frame_get_header_length(mac_frame_t *frame)
{
    return (uint8_t)(mac_frame_get_payload(frame) - mac_frame_get_psdu(frame));
}

uint8_t
mac_frame_get_footer_length(mac_frame_t *frame)
{
    uint8_t footer_length = 0;
    uint8_t index = frame_find_security_header_index(frame);
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX);
    switch ((mac_frame_get_psdu(frame) + index)[0] & MAC_FRAME_SEC_LEVEL_MASK) {
    case MAC_FRAME_SEC_NONE:
    case MAC_FRAME_SEC_ENC:
        footer_length += MAC_FRAME_MIC_0_SIZE;
        break;
    case MAC_FRAME_SEC_MIC_32:
    case MAC_FRAME_SEC_ENC_MIC_32:
        footer_length += MAC_FRAME_MIC_32_SIZE;
        break;
    case MAC_FRAME_SEC_MIC_64:
    case MAC_FRAME_SEC_ENC_MIC_64:
        footer_length += MAC_FRAME_MIC_64_SIZE;
        break;

    case MAC_FRAME_SEC_MIC_128:
    case MAC_FRAME_SEC_ENC_MIC_128:
        footer_length += MAC_FRAME_MIC_128_SIZE;
        break;
    }

exit:
    // frame check sequence
    footer_length += MAC_FRAME_FCS_SIZE;
    return footer_length;
}

uint8_t
mac_frame_get_payload_length(mac_frame_t *frame)
{
    return mac_frame_get_psdu_length(frame) - (mac_frame_get_header_length(frame) + mac_frame_get_footer_length(frame));
}

uint8_t
mac_frame_get_max_payload_length(mac_frame_t *frame)
{
    return MAC_FRAME_MTU - (mac_frame_get_header_length(frame) + mac_frame_get_footer_length(frame));
}

ns_error_t
mac_frame_set_payload_length(mac_frame_t *frame, uint8_t length)
{
    mac_frame_set_psdu_length(frame, mac_frame_get_header_length(frame) + mac_frame_get_footer_length(frame) + length);
    return NS_ERROR_NONE;
}

uint8_t
mac_frame_get_channel(mac_frame_t *frame)
{
    return frame->channel;
}

void
mac_frame_set_channel(mac_frame_t *frame, uint8_t channel)
{
    frame->channel = channel;
}

int8_t
mac_frame_get_rssi(mac_frame_t *frame)
{
    return frame->info.rx_info.rssi;
}

void
mac_frame_set_rssi(mac_frame_t *frame, int8_t rssi)
{
    frame->info.rx_info.rssi = rssi;
}

uint8_t
mac_frame_get_lqi(mac_frame_t *frame)
{
    return frame->info.rx_info.lqi;
}

void
mac_frame_set_lqi(mac_frame_t *frame, uint8_t lqi)
{
    frame->info.rx_info.lqi = lqi;
}

uint8_t
mac_frame_get_max_csma_backoffs(mac_frame_t *frame)
{
    return frame->info.tx_info.max_csma_backoffs;
}

void
mac_frame_set_max_csma_backoffs(mac_frame_t *frame, uint8_t max_csma_backoffs)
{
    frame->info.tx_info.max_csma_backoffs = max_csma_backoffs;
}

uint8_t
mac_frame_get_max_frame_retries(mac_frame_t *frame)
{
    return frame->info.tx_info.max_frame_retries;
}

void
mac_frame_set_max_frame_retries(mac_frame_t *frame, uint8_t max_frame_retries)
{
    frame->info.tx_info.max_frame_retries = max_frame_retries;
}

bool
mac_frame_is_a_retransmission(mac_frame_t *frame)
{
    return frame->info.tx_info.is_a_retx;
}

void
mac_frame_set_is_a_retransmission(mac_frame_t *frame, bool is_a_retx)
{
    frame->info.tx_info.is_a_retx = is_a_retx;
}

void
mac_frame_set_did_tx(mac_frame_t *frame, bool didtx)
{
    frame->didtx = didtx;
}

void
mac_frame_set_csma_ca_enabled(mac_frame_t *frame, bool csma_ca_enabled)
{
    frame->info.tx_info.csma_ca_enabled = csma_ca_enabled;
}

bool
mac_frame_is_csma_ca_enabled(mac_frame_t *frame)
{
    return frame->info.tx_info.csma_ca_enabled;
}

const uint8_t *
mac_frame_get_aes_key(mac_frame_t *frame)
{
    return frame->info.tx_info.aes_key;
}

void
mac_frame_set_aes_key(mac_frame_t *frame, const uint8_t *aes_key)
{
    frame->info.tx_info.aes_key = aes_key;
}

uint8_t
mac_frame_get_psdu_length(mac_frame_t *frame)
{
    return frame->length;
}

void
mac_frame_set_psdu_length(mac_frame_t *frame, uint8_t length)
{
    frame->length = length;
}

uint8_t *
mac_frame_get_psdu(mac_frame_t *frame)
{
    return frame->psdu;
}

uint8_t *
mac_frame_get_header(mac_frame_t *frame)
{
    return mac_frame_get_psdu(frame);
}

uint8_t *
mac_frame_get_payload(mac_frame_t *frame)
{
    uint8_t index = frame_find_payload_index(frame);
    uint8_t *payload = mac_frame_get_psdu(frame) + index;
    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, payload = NULL);
exit:
    return payload;
}

uint8_t *
mac_frame_get_footer(mac_frame_t *frame)
{
    return mac_frame_get_psdu(frame) + mac_frame_get_psdu_length(frame) - mac_frame_get_footer_length(frame);
}

#if NS_CONFIG_ENABLE_TIME_SYNC
void
mac_frame_set_time_ie_offset(mac_frame_t *frame, uint8_t offset)
{
    frame->ie_info->time_ie_offset = offset;
}

void
mac_frame_set_network_time_offset(mac_frame_t *frame, int64_t network_time_offset)
{
    frame->ie_info->network_time_offset = network_time_offset;
}

int64_t
mac_frame_get_network_time_offset(mac_frame_t *frame)
{
    return frame->ie_info->network_time_offset;
}

void
mac_frame_set_time_sync_seq(mac_frame_t *frame, uint8_t time_sync_seq)
{
    frame->ie_info->time_sync_seq = time_sync_seq;
}

uint8_t
mac_frame_get_time_sync_seq(mac_frame_t *frame)
{
    return frame->ie_info->time_sync_seq;
}

uint64_t
mac_frame_get_time_stamp(mac_frame_t *frame)
{
    return frame->ie_info->timestamp;
}

uint8_t *
mac_frame_get_time_ie(mac_frame_t *frame)
{
    mac_time_ie_t *time_ie = NULL;
    uint8_t *cur = NULL;
    uint8_t oui[MAC_FRAME_VENDOR_OUI_SIZE] = {
        MAC_FRAME_VENDOR_OUI_NEST & 0xff,
        (MAC_FRAME_VENDOR_OUI_NEST >> 8) & 0xff,
        (MAC_FRAME_VENDOR_OUI_NEST >> 16) & 0xff
    };

    cur = mac_frame_get_header_ie(frame, MAC_FRAME_HEADER_IE_VENDOR);
    VERIFY_OR_EXIT(cur != NULL);

    cur += sizeof(header_ie_t);
    time_ie = (mac_time_ie_t *)cur;

    VERIFY_OR_EXIT(memcmp(oui,
                          mac_vendor_ie_header_get_vendor_oui(&time_ie->vendor_ie_header),
                          MAC_FRAME_VENDOR_OUI_SIZE) == 0,
                   cur = NULL);

    VERIFY_OR_EXIT(mac_vendor_ie_header_get_sub_type(&time_ie->vendor_ie_header) == MAC_FRAME_VENDOR_IE_TIME,
                   cur = NULL);

exit:
    return cur;
}
#endif // NS_CONFIG_ENABLE_TIME_SYNC

#if NS_CONFIG_HEADER_IE_SUPPORT
ns_error_t
mac_frame_append_header_ie(mac_frame_t *frame, header_ie_t *ie_list, uint8_t ie_count)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = frame_find_header_ie_index(frame);
    uint8_t *cur;
    uint8_t *base;

    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX, error = NS_ERROR_FAILED);
    cur = mac_frame_get_psdu(frame) + index;
    base = cur;

    for (uint8_t i = 0; i < ie_count; i++) {
        memcpy(cur, &ie_list[i].ie, sizeof(header_ie_t));
        cur += sizeof(header_ie_t);
        cur += mac_header_ie_get_length(&ie_list[i]);
    }

    mac_frame_set_psdu_length(frame, mac_frame_get_psdu_length(frame) + (uint8_t)(cur - base));

exit:
    return error;
}

uint8_t *
mac_frame_get_header_ie(mac_frame_t *frame, uint8_t ie_id)
{
    uint8_t index = frame_find_header_ie_index(frame);
    uint8_t *cur = NULL;
    uint8_t *payload = mac_frame_get_payload(frame);

    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX);

    cur = mac_frame_get_psdu(frame) + index;
     
    while (cur + sizeof(header_ie_t) <= payload) {
        header_ie_t *ie = (header_ie_t *)cur;
        uint8_t len = (uint8_t)mac_header_ie_get_length(ie);
        if (mac_header_ie_get_id(ie) == ie_id) {
            break;
        }
        cur += sizeof(header_ie_t);
        VERIFY_OR_EXIT(cur + len <= payload, cur = NULL);
        cur += len;
    }
    if (cur == payload) {
        cur = NULL;
    }
exit:
    return cur;
}
#endif // NS_CONFIG_HEADER_IE_SUPPORT

// --- MAC beacon functions
void
mac_beacon_init(mac_beacon_t *beacon)
{
    beacon->super_frame_spec = encoding_little_endian_swap16(MAC_BEACON_SUPER_FRAME_SPEC);
    beacon->gts_spec = 0;
    beacon->pending_addr_spec = 0;
}

bool
mac_beacon_is_valid(mac_beacon_t *beacon)
{
    return (beacon->super_frame_spec == encoding_little_endian_swap16(MAC_BEACON_SUPER_FRAME_SPEC)) &&
           (beacon->gts_spec == 0) && (beacon->pending_addr_spec == 0);
}

uint8_t *
mac_beacon_get_payload(mac_beacon_t *beacon)
{
    return (uint8_t *)(beacon + sizeof(*beacon));
}

// --- MAC beacon payload functions
void
mac_beacon_payload_init(mac_beacon_payload_t *beacon_payload)
{
    beacon_payload->protocol_id = MAC_BEACON_PAYLOAD_PROTOCOL_ID;
    beacon_payload->flags = MAC_BEACON_PAYLOAD_PROTOCOL_VERSION << MAC_BEACON_PAYLOAD_VERSION_OFFSET;
}

bool
mac_beacon_payload_is_valid(mac_beacon_payload_t *beacon_payload)
{
    return (beacon_payload->protocol_id == MAC_BEACON_PAYLOAD_PROTOCOL_ID);
}

uint8_t
mac_beacon_payload_get_protocol_id(mac_beacon_payload_t *beacon_payload)
{
    return beacon_payload->protocol_id;
}

uint8_t
mac_beacon_payload_get_protocol_version(mac_beacon_payload_t *beacon_payload)
{
    return (beacon_payload->flags >> MAC_BEACON_PAYLOAD_VERSION_OFFSET);
}

bool
mac_beacon_payload_is_native(mac_beacon_payload_t *beacon_payload)
{
    return (beacon_payload->flags & MAC_BEACON_PAYLOAD_NATIVE_FLAG) != 0;
}

void
mac_beacon_payload_clear_native(mac_beacon_payload_t *beacon_payload)
{
    beacon_payload->flags &= ~MAC_BEACON_PAYLOAD_NATIVE_FLAG;
}

void
mac_beacon_payload_set_native(mac_beacon_payload_t *beacon_payload)
{
    beacon_payload->flags |= MAC_BEACON_PAYLOAD_NATIVE_FLAG;
}

bool
mac_beacon_payload_is_joining_permitted(mac_beacon_payload_t *beacon_payload)
{
    return (beacon_payload->flags & MAC_BEACON_PAYLOAD_JOINING_FLAG) != 0;
}

void
mac_beacon_payload_clear_joining_permitted(mac_beacon_payload_t *beacon_payload)
{
    beacon_payload->flags &= ~MAC_BEACON_PAYLOAD_JOINING_FLAG;
}

void
mac_beacon_payload_set_joining_permitted(mac_beacon_payload_t *beacon_payload)
{
    beacon_payload->flags |= MAC_BEACON_PAYLOAD_JOINING_FLAG;
#if NS_CONFIG_JOIN_BEACON_VERSION != MAC_BEACON_PAYLOAD_PROTOCOL_VERSION
    beacon_payload->flags &= ~MAC_BEACON_PAYLOAD_VERSION_MASK;
    beacon_payload->flags |= NS_CONFIG_JOIN_BEACON_VERSION << MAC_BEACON_PAYLOAD_VERSION_OFFSET;
#endif
}

const char *
mac_beacon_payload_get_network_name(mac_beacon_payload_t *beacon_payload)
{
    return beacon_payload->network_name;
}

void
mac_beacon_payload_set_network_name(mac_beacon_payload_t *beacon_payload, const char *network_name)
{
    size_t length = strnlen(network_name, sizeof(beacon_payload->network_name));
    memset(beacon_payload->network_name, 0, sizeof(beacon_payload->network_name));
    memcpy(beacon_payload->network_name, network_name, length);
}

const uint8_t *
mac_beacon_payload_get_extended_panid(mac_beacon_payload_t *beacon_payload)
{
    return beacon_payload->extended_panid;
}

void
mac_beacon_payload_set_extended_panid(mac_beacon_payload_t *beacon_payload, const uint8_t *ext_panid)
{
    memcpy(beacon_payload->extended_panid, ext_panid, sizeof(beacon_payload->extended_panid));
}

#if NS_CONFIG_ENABLE_TIME_SYNC
// --- MAC vendor ie header functions
const uint8_t *
mac_vendor_ie_header_get_vendor_oui(mac_vendor_ie_header_t *vendor_ie_header)
{
    return vendor_ie_header->vendor_oui;
}

void
mac_vendor_ie_header_set_vendor_oui(mac_vendor_ie_header_t *vendor_ie_header, uint8_t *vendor_oui)
{
    memcpy((uint8_t *)&vendor_ie_header->vendor_oui, vendor_oui, MAC_FRAME_VENDOR_OUI_SIZE);
}

uint8_t
mac_vendor_ie_header_get_sub_type(mac_vendor_ie_header_t *vendor_ie_header)
{
    return vendor_ie_header->sub_type;
}

void
mac_vendor_ie_header_set_sub_type(mac_vendor_ie_header_t *vendor_ie_header, uint8_t sub_type)
{
    vendor_ie_header->sub_type = sub_type;
}

// --- MAC time ie funtions
void
mac_time_ie_init(mac_time_ie_t *mac_time_ie)
{
    uint8_t oui[MAC_FRAME_VENDOR_OUI_SIZE] = {
        MAC_FRAME_VENDOR_OUI_NEST & 0xff,
        (MAC_FRAME_VENDOR_OUI_NEST >> 8) & 0xff,
        (MAC_FRAME_VENDOR_OUI_NEST >> 16) & 0xff
    };

    mac_vendor_ie_header_set_vendor_oui(&mac_time_ie->vendor_ie_header, oui);
    mac_vendor_ie_header_set_sub_type(&mac_time_ie->vendor_ie_header, MAC_FRAME_VENDOR_IE_TIME);
}

uint8_t
mac_time_ie_get_sequence(mac_time_ie_t *mac_time_ie)
{
    return mac_time_ie->sequence;
}

void
mac_time_ie_set_sequence(mac_time_ie_t *mac_time_ie, uint8_t sequence)
{
    mac_time_ie->sequence = sequence;
}

uint64_t
mac_time_ie_get_time(mac_time_ie_t *mac_time_ie)
{
    return encoding_little_endian_swap64(mac_time_ie->time);
}

void
mac_time_ie_set_time(mac_time_ie_t *mac_time_ie, uint64_t time)
{
    mac_time_ie->time = encoding_little_endian_swap64(time);
}
#endif // NS_CONFIG_ENABLE_TIME_SYNC

// --- MAC frame private functions
uint16_t
frame_get_frame_control_field(mac_frame_t *frame)
{
    return encoding_little_endian_read_uint16(mac_frame_get_psdu(frame));
}

uint8_t
frame_find_dst_panid_index(mac_frame_t *frame)
{
    uint8_t index = 0;
    VERIFY_OR_EXIT((frame_get_frame_control_field(frame) & MAC_FRAME_FCF_DST_ADDR_MASK) != MAC_FRAME_FCF_DST_ADDR_NONE, index = MAC_FRAME_INVALID_INDEX);

    // frame control field and sequence number
    index = MAC_FRAME_FCF_SIZE + MAC_FRAME_DSN_SIZE;
exit:
    return index;
}

uint8_t
frame_find_dst_addr_index(mac_frame_t *frame)
{
    return MAC_FRAME_FCF_SIZE + MAC_FRAME_DSN_SIZE + sizeof(panid_t);
}

uint8_t
frame_find_src_panid_index(mac_frame_t *frame)
{
    uint8_t index = 0;
    uint16_t fcf = frame_get_frame_control_field(frame);

    VERIFY_OR_EXIT((fcf & MAC_FRAME_FCF_DST_ADDR_MASK) != MAC_FRAME_FCF_DST_ADDR_NONE ||
                   (fcf & MAC_FRAME_FCF_SRC_ADDR_MASK) != MAC_FRAME_FCF_DST_ADDR_NONE,
                   index = MAC_FRAME_INVALID_INDEX);

    // frame control field and sequence number
    index += MAC_FRAME_FCF_SIZE + MAC_FRAME_DSN_SIZE;

    if ((fcf & MAC_FRAME_FCF_PANID_COMPRESSION) == 0) {
        // destination PAN + address
        switch (fcf & MAC_FRAME_FCF_DST_ADDR_MASK) {
        case MAC_FRAME_FCF_DST_ADDR_SHORT:
            index += sizeof(panid_t) + sizeof(short_addr_t);
            break;
        case MAC_FRAME_FCF_DST_ADDR_EXT:
            index += sizeof(panid_t) + sizeof(ext_addr_t);
            break;
        }
    }
exit:
    return index;
}

uint8_t
frame_find_src_addr_index(mac_frame_t *frame)
{
    uint8_t index = 0;
    uint16_t fcf = frame_get_frame_control_field(frame);

    // frame control field and sequence number
    index += MAC_FRAME_FCF_SIZE + MAC_FRAME_DSN_SIZE;

    // destination PAN + address
    switch (fcf & MAC_FRAME_FCF_DST_ADDR_MASK) {
    case MAC_FRAME_FCF_DST_ADDR_SHORT:
        index += sizeof(panid_t) + sizeof(short_addr_t);
        break;
    case MAC_FRAME_FCF_DST_ADDR_EXT:
        index += sizeof(panid_t) + sizeof(ext_addr_t);
        break;
    }

    // source PAN
    if (mac_frame_is_src_panid_present(frame, fcf)) {
        index += sizeof(panid_t);
    }

    return index;
}

uint8_t
frame_find_security_header_index(mac_frame_t *frame)
{
    uint8_t index = 0;
    uint16_t fcf = frame_get_frame_control_field(frame);

    VERIFY_OR_EXIT((fcf & MAC_FRAME_FCF_SECURITY_ENABLED) != 0, index = MAC_FRAME_INVALID_INDEX);

    // frame control field and sequence number
    index += MAC_FRAME_FCF_SIZE + MAC_FRAME_DSN_SIZE;

    // destination PAN + address
    switch (fcf & MAC_FRAME_FCF_DST_ADDR_MASK) {
    case MAC_FRAME_FCF_DST_ADDR_SHORT:
        index += sizeof(panid_t) + sizeof(short_addr_t);
        break;
    case MAC_FRAME_FCF_DST_ADDR_EXT:
        index += sizeof(panid_t) + sizeof(ext_addr_t);
        break;
    }

    // source PAN
    if (mac_frame_is_src_panid_present(frame, fcf)) {
        index += sizeof(panid_t);
    }

    // source address
    switch (fcf & MAC_FRAME_FCF_SRC_ADDR_MASK) {
    case MAC_FRAME_FCF_SRC_ADDR_SHORT:
        index += sizeof(short_addr_t);
        break;
    case MAC_FRAME_FCF_SRC_ADDR_EXT:
        index += sizeof(ext_addr_t);
        break;
    }

exit:
    return index;
}

uint8_t
frame_skip_security_header_index(mac_frame_t *frame)
{
    uint8_t index = 0;
    uint16_t fcf;

    // frame control
    index += MAC_FRAME_FCF_SIZE;
    // sequence number
    index += MAC_FRAME_DSN_SIZE;

    VERIFY_OR_EXIT((index + MAC_FRAME_FCS_SIZE) <= mac_frame_get_psdu_length(frame),
                   index = MAC_FRAME_INVALID_INDEX);

    fcf = frame_get_frame_control_field(frame);

    // destination PAN + address
    switch (fcf & MAC_FRAME_FCF_DST_ADDR_MASK) {
    case MAC_FRAME_FCF_DST_ADDR_NONE:
        break;
    case MAC_FRAME_FCF_DST_ADDR_SHORT:
        index += sizeof(panid_t) + sizeof(short_addr_t);
        break;
    case MAC_FRAME_FCF_DST_ADDR_EXT:
        index += sizeof(panid_t) + sizeof(ext_addr_t);
        break;
    default:
        EXIT_NOW(index = MAC_FRAME_INVALID_INDEX);
    }

    // source PAN
    if (mac_frame_is_src_panid_present(frame, fcf)) {
        index += sizeof(panid_t);
    }

    // source address
    switch (fcf & MAC_FRAME_FCF_SRC_ADDR_MASK) {
    case MAC_FRAME_FCF_SRC_ADDR_NONE:
        break;
    case MAC_FRAME_FCF_SRC_ADDR_SHORT:
        index += sizeof(short_addr_t);
        break;
    case MAC_FRAME_FCF_SRC_ADDR_EXT:
        index += sizeof(ext_addr_t);
        break;
    default:
        EXIT_NOW(index = MAC_FRAME_INVALID_INDEX);
    }

    VERIFY_OR_EXIT((index + MAC_FRAME_FCS_SIZE) <= mac_frame_get_psdu_length(frame),
                   index = MAC_FRAME_INVALID_INDEX);

    // security control + frame counter + key identifier
    if ((fcf & MAC_FRAME_FCF_SECURITY_ENABLED) != 0) {
        uint8_t security_control = *(mac_frame_get_psdu(frame) + index);
        index += MAC_FRAME_SECURITY_CONTROL_SIZE + MAC_FRAME_FRAME_COUNTER_SIZE;
        switch (security_control & MAC_FRAME_KEY_ID_MODE_MASK) {
        case MAC_FRAME_KEY_ID_MODE_0:
            index += MAC_FRAME_KEY_SOURCE_SIZE_MODE_0;
            break;
        case MAC_FRAME_KEY_ID_MODE_1:
            index += MAC_FRAME_KEY_SOURCE_SIZE_MODE_1 + MAC_FRAME_KEY_INDEX_SIZE;
            break;
        case MAC_FRAME_KEY_ID_MODE_2:
            index += MAC_FRAME_KEY_SOURCE_SIZE_MODE_2 + MAC_FRAME_KEY_INDEX_SIZE;
            break;
        case MAC_FRAME_KEY_ID_MODE_3:
            index += MAC_FRAME_KEY_SOURCE_SIZE_MODE_3 + MAC_FRAME_KEY_INDEX_SIZE;
            break;
        }
    }

exit:
    return index;
}

uint8_t
frame_find_payload_index(mac_frame_t *frame)
{
    uint8_t index = frame_skip_security_header_index(frame);
#if NS_CONFIG_HEADER_IE_SUPPORT
    uint8_t *cur = NULL;
    uint8_t *footer = mac_frame_get_footer(frame);
#endif // NS_CONFIG_HEADER_IE_SUPPORT

    VERIFY_OR_EXIT(index != MAC_FRAME_INVALID_INDEX);

#if NS_CONFIG_HEADER_IE_SUPPORT
    cur = mac_frame_get_psdu(frame) + index;
    if (mac_frame_is_ie_present(frame)) {
        while (cur + sizeof(header_ie_t) <= footer) {
            header_ie_t *ie = (header_ie_t *)cur;
            uint8_t len = (uint8_t)mac_header_ie_get_length(ie);
            cur += sizeof(header_ie_t);
            index += sizeof(header_ie_t);
            VERIFY_OR_EXIT(cur + len <= footer, index = MAC_FRAME_INVALID_INDEX);
            cur += len;
            index += len;
            if (mac_header_ie_get_length(ie) == MAC_FRAME_HEADER_IE_TERMINATION_2) {
                break;
            }
        }
        // assume no payload IE in current implementation
    }
#endif // NS_CONFIG_HEADER_IE_SUPPORT

    // command ID
    if ((frame_get_frame_control_field(frame) & MAC_FRAME_FCF_FRAME_TYPE_MASK) == MAC_FRAME_FCF_FRAME_MAC_CMD) {
        index += MAC_FRAME_COMMAND_ID_SIZE;
    }

exit:
    return index;
}

#if NS_CONFIG_HEADER_IE_SUPPORT
uint8_t
frame_find_header_ie_index(mac_frame_t *frame)
{
    uint8_t index;
    VERIFY_OR_EXIT(mac_frame_is_ie_present(frame), index = MAC_FRAME_INVALID_INDEX);
    index = frame_skip_security_header_index(frame);
exit:
    return index;
}
#endif // NS_CONFIG_HEADER_IE_SUPPORT
