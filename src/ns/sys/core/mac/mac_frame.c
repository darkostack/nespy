#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/random.h"
#include "ns/sys/core/common/encoding.h"
#include "ns/sys/core/mac/mac_frame.h"
#include <string.h>

STRING(mac_ext_addr_info, MAC_EXT_ADDR_INFO_STRING_SIZE);
STRING(mac_addr_info, sizeof(mac_ext_addr_info_string));

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
mac_addr_set_extended_from_buf(mac_addr_t *mac_addr, const uint8_t *buf, bool reverse)
{
    mac_addr->type = MAC_ADDR_TYPE_EXTENDED;
    if (reverse) {
        for (unsigned int i = 0; i < sizeof(ext_addr_t); i++) {
            mac_addr->shared.ext_addr.m8[i] = buf[sizeof(ext_addr_t) - 1 - i];
        }
    } else {
        memcpy(mac_addr->shared.ext_addr.m8, buf, sizeof(ext_addr_t));
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
