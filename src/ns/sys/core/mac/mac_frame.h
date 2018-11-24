#ifndef NS_CORE_MAC_MAC_FRAME_H_
#define NS_CORE_MAC_MAC_FRAME_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/platform/radio.h"

typedef ns_panid_t panid_t;
typedef ns_shortaddr_t shortaddr_t;
typedef struct _macaddr macaddr_t;

enum {
    MAC_SHORT_ADDR_BROADCAST = 0xffff,
    MAC_SHORT_ADDR_INVALID = 0xfffe,
    MAC_PANID_BROADCAST = 0xffff,
};

enum {
    MAC_EXTADDR_GROUP_FLAG = 1 << 0,
    MAC_EXTADDR_LOCAL_FLAG = 1 << 1,
};

typedef enum _macaddr_type {
    MAC_ADDR_TYPE_NONE,
    MAC_ADDR_TYPE_SHORT,
    MAC_ADDR_TYPE_EXTENDED,
} macaddr_type_t;

struct _macaddr {
    union {
        shortaddr_t shortaddr;
        extaddr_t extaddr;
    } shared;
    macaddr_type_t type;
};

// --- MAC extended address functions
void
mac_extaddr_gen_random(extaddr_t *extaddr);

bool
mac_extaddr_is_group(extaddr_t *extaddr);

void
mac_extaddr_set_group(extaddr_t *extaddr, bool group);

void
mac_extaddr_toggle_group(extaddr_t *extaddr);

bool
mac_extaddr_is_local(extaddr_t *extaddr);

void
mac_extaddr_set_local(extaddr_t *extaddr, bool local);

void
mac_extaddr_toggle_local(extaddr_t *extaddr);

bool
mac_extaddr_is_equal(extaddr_t *extaddr1, extaddr_t *extaddr2);

// --- MAC address functions
void
mac_addr_make_new(macaddr_t *macaddr);

macaddr_type_t
mac_addr_get_type(macaddr_t *macaddr);

bool
mac_addr_type_is_none(macaddr_t *macaddr);

bool
mac_addr_type_is_short(macaddr_t *macaddr);

bool
mac_addr_type_is_extended(macaddr_t *macaddr);

shortaddr_t
mac_addr_get_short(macaddr_t *macaddr);

extaddr_t
mac_addr_get_extended(macaddr_t *macaddr);

void
mac_addr_set_none(macaddr_t *macaddr);

void
mac_addr_set_short(macaddr_t *macaddr, shortaddr_t *shortaddr);

void
mac_addr_set_extended(macaddr_t *macaddr, extaddr_t *extaddr);

void
mac_addr_set_extended_from_buf(macaddr_t *macaddr, const uint8_t *buf, bool reverse);

void
mac_addr_is_broadcast(macaddr_t *macaddr);

void
mac_addr_is_short_addr_invalid(macaddr_t *macaddr);

#endif // NS_CORE_MAC_MAC_FRAME_H_
