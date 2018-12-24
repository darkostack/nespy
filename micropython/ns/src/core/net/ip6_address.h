#ifndef NS_CORE_NET_IP6_ADDRESS_H_
#define NS_CORE_NET_IP6_ADDRESS_H_

#include "core/core-config.h"
#include "core/common/string.h"
#include "core/mac/mac_frame.h"
#include <stdint.h>
#include <string.h>

typedef ns_ip6_addr_t ip6_addr_t;

enum {
    IP6_ADDR_ALOC16_MASK = 0xfc,
    IP6_ADDR_RLOC16_RESERVED_BIT_MASK = 0x02,
};

enum {
    IP6_ADDR_INTERFACE_IDENTIFIER_SIZE = 8,
    IP6_ADDR_STRING_SIZE = 40,
    IP6_ADDR_MESH_LOCAL_PREFIX_LENGTH = 64,
    IP6_ADDR_MESH_LOCAL_PREFIX_SIZE = 8,
};

enum {
    IP6_ADDR_NODE_LOCAL_SCOPE      = 0,
    IP6_ADDR_INTERFACE_LOCAL_SCOPE = 1,
    IP6_ADDR_LINK_LOCAL_SCOPE      = 2,
    IP6_ADDR_REALM_LOCAL_SCOPE     = 3,
    IP6_ADDR_ADMIN_LOCAL_SCOPE     = 4,
    IP6_ADDR_SITE_LOCAL_SCOPE      = 5,
    IP6_ADDR_ORG_LOCAL_SCOPE       = 8,
    IP6_ADDR_GLOBAL_SCOPE          = 14,
};

enum {
    IP6_ADDR_INTERFACE_IDENTIFIER_OFFSET = 8,
};

void
ip6_addr_clear(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_unspecified(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_loopback(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_interface_local(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_link_local(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_multicast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_link_local_multicast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_link_local_all_nodes_multicast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_link_local_all_routers_multicast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_realm_local_multicast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_realm_local_all_nodes_multicast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_realm_local_all_routers_multicast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_realm_local_all_mpl_forwarders(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_multicast_larger_than_realm_local(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_routing_locator(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_anycast_routing_locator(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_subnet_router_anycast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_reserved_subnet_anycast(ip6_addr_t *ip6_addr);

bool
ip6_addr_is_iid_reserved(ip6_addr_t *ip6_addr);

uint8_t *
ip6_addr_get_iid(ip6_addr_t *ip6_addr);

void
ip6_addr_set_iid(ip6_addr_t *ip6_addr, const uint8_t *iid);

void
ip6_addr_set_iid_from_mac_ext_addr(ip6_addr_t *ip6_addr, mac_ext_addr_t *ext_addr);

void
ip6_addr_to_ext_addr_from_ext_addr(ip6_addr_t *ip6_addr, mac_ext_addr_t *ext_addr);

void
ip6_addr_to_ext_addr_from_mac_addr(ip6_addr_t *ip6_addr, mac_addr_t *mac_addr);

uint8_t
ip6_addr_get_scope(ip6_addr_t *ip6_addr);

uint8_t
ip6_addr_prefix_match(ip6_addr_t *ip6_addr, const ip6_addr_t *other);

bool
ip6_addr_is_equal(ip6_addr_t *ip6_addr, const ip6_addr_t *other);

ns_error_t
ip6_addr_from_string(ip6_addr_t *ip6_addr, const char *buf);

string_t *
ip6_addr_to_string(ip6_addr_t *ip6_addr);

#endif // NS_CORE_NET_IP6_ADDRESS_H_
