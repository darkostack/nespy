#ifndef NS_CORE_NET_NETIF_H_
#define NS_CORE_NET_NETIF_H_

#include "core/core-config.h"
#include "core/common/instance.h"
#include "core/common/message.h"
#include "core/common/tasklet.h"
#include "core/mac/mac_frame.h"
#include "core/net/ip6_address.h"
#include "core/net/socket.h"

typedef enum _ip6_link_addr_hardware_type {
    IP6_LINK_ADDR_EUI64 = 27,
} ip6_link_addr_hardware_type_t;

typedef struct _ip6_link_addr {
    ip6_link_addr_hardware_type_t type;
    uint8_t length;
    mac_ext_addr_t ext_addr;
} ip6_link_addr_t;

typedef ns_netif_addr_t ip6_netif_unicast_addr_t;
typedef ns_netif_multicast_addr_t ip6_netif_multicast_addr_t;

enum {
    IP6_NETIF_MULTICAST_PREFIX_LENGTH = 128,
};

typedef struct _ip6_netif ip6_netif_t;
struct _ip6_netif {
    void *instance;
    ip6_netif_unicast_addr_t *unicast_addrs;
    ip6_netif_multicast_addr_t *multicast_addrs;
    int8_t interface_id;
    bool multicast_promiscuous;
    ip6_netif_t *next;

    ns_ip6_addr_callback_func_t addr_callback;
    void *addr_callback_context;

    ip6_netif_unicast_addr_t ext_unicast_addrs[NS_CONFIG_MAX_EXT_IP_ADDRS];
    ip6_netif_multicast_addr_t ext_multicast_addrs[NS_CONFIG_MAX_EXT_MULTICAST_IP_ADDRS];

    ns_netif_multicast_addr_t realm_local_all_mpl_forwarders_multicast_addr;
    ns_netif_multicast_addr_t link_local_all_nodes_multicast_addr;
    ns_netif_multicast_addr_t realm_local_all_nodes_multicast_addr;
    ns_netif_multicast_addr_t link_local_all_routers_multicast_addr;
    ns_netif_multicast_addr_t realm_local_all_routers_multicast_addr;
};

// --- Ip6 Netif unicast address
ip6_addr_t *
ip6_netif_unicast_addr_get_addr(ip6_netif_unicast_addr_t *netif_unicast_addr);

uint8_t
ip6_netif_unicast_addr_get_scope(ip6_netif_unicast_addr_t *netif_unicast_addr);

ip6_netif_unicast_addr_t *
ip6_netif_unicast_addr_get_next(ip6_netif_unicast_addr_t *netif_unicast_addr);

// --- Ip6 Netif multicast address
ip6_addr_t *
ip6_netif_multicast_addr_get_addr(ip6_netif_multicast_addr_t *netif_multicast_addr);

ip6_netif_multicast_addr_t *
ip6_netif_multicast_addr_get_next(ip6_netif_multicast_addr_t *netif_multicast_addr);

// --- Ip6 Netif
void
ip6_netif_ctor(ip6_netif_t *ip6_netif, void *instance, int8_t interface_id);

ip6_netif_t *
ip6_netif_get_next(ip6_netif_t *ip6_netif);

int8_t
ip6_netif_get_interface_id(ip6_netif_t *ip6_netif);

void
ip6_netif_set_addr_callback(ip6_netif_t *ip6_netif, ns_ip6_addr_callback_func_t callback, void *callback_context);

ip6_netif_unicast_addr_t *
ip6_netif_get_unicast_addrs(ip6_netif_t *ip6_netif);

ns_error_t
ip6_netif_add_unicast_addr(ip6_netif_t *ip6_netif, ip6_netif_unicast_addr_t *addr);

ns_error_t
ip6_netif_remove_unicast_addr(ip6_netif_t *ip6_netif, ip6_netif_unicast_addr_t *addr);

ns_error_t
ip6_netif_add_external_unicast_addr(ip6_netif_t *ip6_netif, ip6_netif_unicast_addr_t *addr);

ns_error_t
ip6_netif_remove_external_unicast_addr(ip6_netif_t *ip6_netif, ip6_addr_t *addr);

void
ip6_netif_remove_all_external_unicast_addrs(ip6_netif_t *ip6_netif);

bool
ip6_netif_is_unicast_addr(ip6_netif_t *ip6_netif, const ip6_addr_t *addr);

bool
ip6_netif_is_multicast_subscribed(ip6_netif_t *ip6_netif, const ip6_addr_t *addr);

ns_error_t
ip6_netif_subscribe_all_routers_multicast(ip6_netif_t *ip6_netif);

ns_error_t
ip6_netif_unsubscribe_all_routers_multicast(ip6_netif_t *ip6_netif);

const ip6_netif_multicast_addr_t *
ip6_netif_get_multicast_addrs(ip6_netif_t *ip6_netif);

ns_error_t
ip6_netif_subscribe_multicast(ip6_netif_t *ip6_netif, ip6_netif_multicast_addr_t *addr);

ns_error_t
ip6_netif_unsubscribe_multicast(ip6_netif_t *ip6_netif, const ip6_netif_multicast_addr_t *addr);

ns_error_t
ip6_netif_get_next_external_multicast(ip6_netif_t *ip6_netif, uint8_t *iterator, ip6_addr_t *addr);

ns_error_t
ip6_netif_subscribe_external_multicast(ip6_netif_t *ip6_netif, const ip6_addr_t *addr);

ns_error_t
ip6_netif_unsubscribe_external_multicast(ip6_netif_t *ip6_netif, const ip6_addr_t *addr);

void
ip6_netif_unsubscribe_all_external_multicast(ip6_netif_t *ip6_netif);

bool
ip6_netif_is_multicast_promiscuous_enabled(ip6_netif_t *ip6_netif);

void
ip6_netif_set_multicast_promiscuous(ip6_netif_t *ip6_netif, bool enabled);

ns_error_t
ip6_netif_send_message(ip6_netif_t *ip6_netif, message_t message);

ns_error_t
ip6_netif_get_link_addr(ip6_netif_t *ip6_netif, ip6_link_addr_t *addr);

ns_error_t
ip6_netif_route_lookup(ip6_netif_t *ip6_netif,
                       const ip6_addr_t *source,
                       const ip6_addr_t *destination,
                       uint8_t *prefix_match);

void
ip6_netif_subscribe_all_nodes_multicast(ip6_netif_t *ip6_netif);

void
ip6_netif_unsubscribe_all_nodes_multicast(ip6_netif_t *ip6_netif);

#endif // NS_CORE_NET_NETIF_H_
