#include "core/net/ip6_address.h"
#include "core/common/string.h"
#include "core/common/encoding.h"
#include "core/common/code_utils.h"
#include "core/common/instance.h"
#include <stdio.h>

STRING(ip6_addr_info, IP6_ADDR_STRING_SIZE);

// --- private funtions declarations
static uint8_t
ip6_addr_is_prefix_match(const uint8_t *prefixa, const uint8_t *prefixb, uint8_t max_length);

// --- ip6 address functions
void
ip6_addr_clear(ip6_addr_t *ip6_addr)
{
    memset(ip6_addr->fields.m8, 0, sizeof(ip6_addr->fields));
}

bool
ip6_addr_is_unspecified(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[0] == 0 &&
            ip6_addr->fields.m32[1] == 0 &&
            ip6_addr->fields.m32[2] == 0 &&
            ip6_addr->fields.m32[3] == 0);
}

bool
ip6_addr_is_loopback(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[0] == 0 &&
            ip6_addr->fields.m32[1] == 0 &&
            ip6_addr->fields.m32[2] == 0 &&
            ip6_addr->fields.m32[3] == encoding_big_endian_swap32(1));
}

bool
ip6_addr_is_link_local(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m8[0] == 0xfe) && ((ip6_addr->fields.m8[1] & 0xc0) == 0x80);
}

bool
ip6_addr_is_multicast(ip6_addr_t *ip6_addr)
{
    return ip6_addr->fields.m8[0] == 0xff;
}

bool
ip6_addr_is_link_local_multicast(ip6_addr_t *ip6_addr)
{
    return ip6_addr_is_multicast(ip6_addr) && (ip6_addr_get_scope(ip6_addr) == IP6_ADDR_LINK_LOCAL_SCOPE);
}

bool
ip6_addr_is_link_local_all_nodes_multicast(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[0] == encoding_big_endian_swap32(0xff020000) &&
            ip6_addr->fields.m32[1] == 0 &&
            ip6_addr->fields.m32[2] == 0 &&
            ip6_addr->fields.m32[3] == encoding_big_endian_swap32(0x01));
}

bool
ip6_addr_is_link_local_all_routers_multicast(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[0] == encoding_big_endian_swap32(0xff020000) &&
            ip6_addr->fields.m32[1] == 0 &&
            ip6_addr->fields.m32[2] == 0 &&
            ip6_addr->fields.m32[3] == encoding_big_endian_swap32(0x02));
}

bool
ip6_addr_is_realm_local_multicast(ip6_addr_t *ip6_addr)
{
    return ip6_addr_is_multicast(ip6_addr) && (ip6_addr_get_scope(ip6_addr) == IP6_ADDR_REALM_LOCAL_SCOPE);
}

bool
ip6_addr_is_realm_local_all_nodes_multicast(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[0] == encoding_big_endian_swap32(0xff030000) &&
            ip6_addr->fields.m32[1] == 0 &&
            ip6_addr->fields.m32[2] == 0 &&
            ip6_addr->fields.m32[3] == encoding_big_endian_swap32(0x01));
}

bool
ip6_addr_is_realm_local_all_routers_multicast(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[0] == encoding_big_endian_swap32(0xff030000) &&
            ip6_addr->fields.m32[1] == 0 &&
            ip6_addr->fields.m32[2] == 0 &&
            ip6_addr->fields.m32[3] == encoding_big_endian_swap32(0x02));
}

bool
ip6_addr_is_realm_local_all_mpl_forwarders(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[0] == encoding_big_endian_swap32(0xff030000) &&
            ip6_addr->fields.m32[1] == 0 &&
            ip6_addr->fields.m32[2] == 0 &&
            ip6_addr->fields.m32[3] == encoding_big_endian_swap32(0xfc));
}

bool
ip6_addr_is_multicast_larger_than_realm_local(ip6_addr_t *ip6_addr)
{
    return ip6_addr_is_multicast(ip6_addr) && (ip6_addr_get_scope(ip6_addr) > IP6_ADDR_REALM_LOCAL_SCOPE);
}

bool
ip6_addr_is_routing_locator(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m16[4] == encoding_big_endian_swap16(0x0000) &&
            ip6_addr->fields.m16[5] == encoding_big_endian_swap16(0x00ff) &&
            ip6_addr->fields.m16[6] == encoding_big_endian_swap16(0xfe00) &&
            ip6_addr->fields.m8[14] < IP6_ADDR_ALOC16_MASK &&
            (ip6_addr->fields.m8[14] & IP6_ADDR_RLOC16_RESERVED_BIT_MASK) == 0);
}

bool
ip6_addr_is_anycast_routing_locator(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m16[4] == encoding_big_endian_swap16(0x0000) &&
            ip6_addr->fields.m16[5] == encoding_big_endian_swap16(0x00ff) &&
            ip6_addr->fields.m16[6] == encoding_big_endian_swap16(0xfe00) &&
            ip6_addr->fields.m8[14] == IP6_ADDR_ALOC16_MASK);
}

bool
ip6_addr_is_subnet_router_anycast(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[2] == 0 && ip6_addr->fields.m32[3] == 0);
}

bool
ip6_addr_is_reserved_subnet_anycast(ip6_addr_t *ip6_addr)
{
    return (ip6_addr->fields.m32[2] == encoding_big_endian_swap32(0xfdffffff) &&
            ip6_addr->fields.m16[6] == 0xffff &&
            ip6_addr->fields.m8[14] == 0xff &&
            ip6_addr->fields.m8[15] >= 0x80);
}

bool
ip6_addr_is_iid_reserved(ip6_addr_t *ip6_addr)
{
    return ip6_addr_is_subnet_router_anycast(ip6_addr) ||
           ip6_addr_is_reserved_subnet_anycast(ip6_addr) ||
           ip6_addr_is_anycast_routing_locator(ip6_addr);
}

uint8_t *
ip6_addr_get_iid(ip6_addr_t *ip6_addr)
{
    return ip6_addr->fields.m8 + IP6_ADDR_INTERFACE_IDENTIFIER_OFFSET;
}

void
ip6_addr_set_iid(ip6_addr_t *ip6_addr, const uint8_t *iid)
{
    memcpy(ip6_addr->fields.m8 + IP6_ADDR_INTERFACE_IDENTIFIER_OFFSET, iid, IP6_ADDR_INTERFACE_IDENTIFIER_SIZE);
}

void
ip6_addr_set_iid_from_mac_ext_addr(ip6_addr_t *ip6_addr, mac_ext_addr_t *ext_addr)
{
    memcpy(ip6_addr->fields.m8 + IP6_ADDR_INTERFACE_IDENTIFIER_OFFSET,
           ext_addr->m8,
           IP6_ADDR_INTERFACE_IDENTIFIER_SIZE);
    ip6_addr->fields.m8[IP6_ADDR_INTERFACE_IDENTIFIER_OFFSET] ^= 0x02;
}

void
ip6_addr_to_ext_addr(ip6_addr_t *ip6_addr, mac_ext_addr_t *ext_addr)
{
    memcpy(ext_addr->m8, ip6_addr->fields.m8 + IP6_ADDR_INTERFACE_IDENTIFIER_OFFSET, sizeof(ext_addr->m8));
    mac_ext_addr_toggle_local(ext_addr);
}

void
ip6_addr_to_mac_addr(ip6_addr_t *ip6_addr, mac_addr_t *mac_addr)
{
    mac_addr_set_extended_from_buffer(mac_addr,
                                      ip6_addr->fields.m8 + IP6_ADDR_INTERFACE_IDENTIFIER_OFFSET,
                                      /* reverse */ false);
    mac_ext_addr_toggle_local(mac_addr_get_extended(mac_addr));
}

uint8_t
ip6_addr_get_scope(ip6_addr_t *ip6_addr)
{
    uint8_t rval;

    if (ip6_addr_is_multicast(ip6_addr)) {
        rval = ip6_addr->fields.m8[1] & 0xf;
    } else if (ip6_addr_is_link_local(ip6_addr)) {
        rval = IP6_ADDR_LINK_LOCAL_SCOPE;
    } else if (ip6_addr_is_loopback(ip6_addr)) {
        rval = IP6_ADDR_NODE_LOCAL_SCOPE;
    } else {
        rval = IP6_ADDR_GLOBAL_SCOPE;
    }

    return rval;
}

uint8_t
ip6_addr_prefix_match(ip6_addr_t *ip6_addr, const ip6_addr_t *other)
{
    return ip6_addr_is_prefix_match(ip6_addr->fields.m8, other->fields.m8, sizeof(ip6_addr_t));
}

bool
ip6_addr_is_equal(ip6_addr_t *ip6_addr, const ip6_addr_t *other)
{
    return memcmp(ip6_addr->fields.m8, other->fields.m8, sizeof(ip6_addr->fields.m8)) == 0;
}

ns_error_t
ip6_addr_from_string(ip6_addr_t *ip6_addr, const char *buf)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t *dst = (uint8_t *)(ip6_addr->fields.m8);
    uint8_t *endp = (uint8_t *)(ip6_addr->fields.m8 + 15);
    uint8_t *colonp = NULL;
    uint16_t val = 0;
    uint8_t count = 0;
    bool first = true;
    char ch;
    uint8_t d;

    memset(ip6_addr->fields.m8, 0, 16);

    dst--;

    for (;;) {
        ch = *buf++;
        d = ch & 0xf;
        if (('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F')) {
            d += 9;
        } else if (ch == ':' || ch == '\0' || ch == ' ') {
            if (count) {
                VERIFY_OR_EXIT(dst + 2 <= endp, error = NS_ERROR_PARSE);
                *(dst + 1) = (uint8_t)(val >> 8);
                *(dst + 2) = (uint8_t)(val);
                dst += 2;
                count = 0;
                val = 0;
            } else if (ch == ':') {
                VERIFY_OR_EXIT(colonp == NULL || first, error = NS_ERROR_PARSE);
                colonp = dst;
            }
            if (ch == '\0' || ch == ' ') {
                break;
            }
            continue;
        } else {
            VERIFY_OR_EXIT('0' <= ch && ch <= '9', error = NS_ERROR_PARSE);
        }

        first = false;
        val = (uint16_t)((val << 4) | d);
        VERIFY_OR_EXIT(++count <= 4, error = NS_ERROR_PARSE);
    }

    while (colonp && dst > colonp) {
        *endp-- = *dst--;
    }

    while (endp > dst) {
        *endp-- = 0;
    }

exit:
    return error;
}

string_t *
ip6_addr_to_string(ip6_addr_t *ip6_addr)
{
    string_t *ip6_addr_string = &ip6_addr_info_string;
    string_clear(ip6_addr_string);
    string_set(ip6_addr_string, "%x:%x:%x:%x:%x:%x:%x:%x",
               encoding_big_endian_swap16(ip6_addr->fields.m16[0]),
               encoding_big_endian_swap16(ip6_addr->fields.m16[1]),
               encoding_big_endian_swap16(ip6_addr->fields.m16[2]),
               encoding_big_endian_swap16(ip6_addr->fields.m16[3]),
               encoding_big_endian_swap16(ip6_addr->fields.m16[4]),
               encoding_big_endian_swap16(ip6_addr->fields.m16[5]),
               encoding_big_endian_swap16(ip6_addr->fields.m16[6]),
               encoding_big_endian_swap16(ip6_addr->fields.m16[7]));
    return ip6_addr_string;
}

// --- private functions
static uint8_t
ip6_addr_is_prefix_match(const uint8_t *prefixa, const uint8_t *prefixb, uint8_t max_length)
{
    uint8_t rval = 0;
    uint8_t diff;

    if (max_length > sizeof(ip6_addr_t)) {
        max_length = sizeof(ip6_addr_t);
    }

    for (uint8_t i = 0; i < max_length; i++) {
        diff = prefixa[i] ^ prefixb[i];
        if (diff == 0) {
            rval += 8;
        } else {
            while ((diff & 0x80) == 0) {
                rval++;
                diff <<= 1;
            }
            break;
        }
    }

    return rval;
}
