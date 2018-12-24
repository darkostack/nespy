#ifndef NS_DHCP6_CLIENT_H_
#define NS_DHCP6_CLIENT_H_

#include "ns/ip6.h"

typedef struct _ns_dhcp_addr {
    ns_netif_addr_t addr;
    uint32_t preferred_lifetime;
    uint32_t valid_lifetime;
} ns_dhcp_addr_t;

void
ns_dhcp6_client_update(ns_instance_t instance, ns_dhcp_addr_t *addrs, uint32_t num_addrs, void *context);

#endif // NS_DHCP6_CLIENT_H_
