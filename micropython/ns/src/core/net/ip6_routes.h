#ifndef NS_CORE_NET_IP6_ROUTES_H_
#define NS_CORE_NET_IP6_ROUTES_H_

#include "core/core-config.h"
#include "core/common/message.h"
#include "core/net/ip6_address.h"

typedef struct _ip6_route ip6_route_t;
struct _ip6_route {
    ip6_addr_t prefix;
    uint8_t prefix_length;
    int8_t interface_id;
    ip6_route_t *next;
};

typedef struct _ip6_routes {
    void *instance;
    ip6_route_t *routes;
} ip6_routes_t;

void
ip6_routes_ctor(ip6_routes_t *ip6_routes, void *instance);

ns_error_t
ip6_routes_add(ip6_routes_t *ip6_routes, ip6_route_t *ip6_route);

ns_error_t
ip6_routes_remove(ip6_routes_t *ip6_routes, ip6_route_t *ip6_route);

int8_t
ip6_routes_lookup(ip6_routes_t *ip6_routes, const ip6_addr_t *source, const ip6_addr_t *destination);

#endif // NS_CORE_NET_IP6_ROUTES_H_
