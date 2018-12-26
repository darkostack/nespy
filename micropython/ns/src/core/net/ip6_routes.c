#include "core/net/ip6_routes.h"
#include "core/common/code_utils.h"
#include "core/common/instance.h"
#include "core/net/ip6.h"
#include "core/net/netif.h"

void
ip6_routes_ctor(ip6_routes_t *ip6_routes, void *instance)
{
    ip6_routes->instance = instance;
    ip6_routes->routes = NULL;
}

ns_error_t
ip6_routes_add(ip6_routes_t *ip6_routes, ip6_route_t *ip6_route)
{
    ns_error_t error = NS_ERROR_NONE;

    for (ip6_route_t *cur = ip6_routes->routes; cur; cur = cur->next) {
        VERIFY_OR_EXIT(cur != ip6_route, error = NS_ERROR_ALREADY);
    }

    ip6_route->next = ip6_routes->routes;
    ip6_routes->routes = ip6_route;

exit:
    return error;
}

ns_error_t
ip6_routes_remove(ip6_routes_t *ip6_routes, ip6_route_t *ip6_route)
{
    if (ip6_route == ip6_routes->routes) {
        ip6_routes->routes = ip6_route->next;
    } else {
        for (ip6_route_t *cur = ip6_routes->routes; cur; cur = cur->next) {
            if (cur->next == ip6_route) {
                cur->next = ip6_route->next;
                break;
            }
        }
    }

    ip6_route->next = NULL;

    return NS_ERROR_NONE;
}

int8_t
ip6_routes_lookup(ip6_routes_t *ip6_routes, const ip6_addr_t *source, const ip6_addr_t *destination)
{
    int8_t max_prefix_match = -1;
    uint8_t prefix_match;
    int8_t rval = -1;

    for (ip6_route_t *cur = ip6_routes->routes; cur; cur = cur->next) {
        prefix_match = ip6_addr_prefix_match(&cur->prefix, destination);
        if (prefix_match < cur->prefix_length) {
            continue;
        }
        if (prefix_match > cur->prefix_length) {
            prefix_match = cur->prefix_length;
        }
        if (max_prefix_match > (int8_t)prefix_match) {
            continue;
        }
        max_prefix_match = (int8_t)prefix_match;
        rval = cur->interface_id;
    }

#if 0 // TODO:
    for (ip6_netif_t *netif = ip6_get_netif_list(instance_get_ip6(ip6_routes->instance)); netif;
         netif = ip6_netif_get_next(netif)) {
        if (ip6_netif_route_lookup(netif, source, destination, &prefix_match) == NS_ERROR_NONE &&
            (int8_t)prefix_match > max_prefix_match) {
            max_prefix_match = (int8_t)prefix_match;
            rval = ip6_netif_get_interface_id(netif);
        }
    }
#endif

    return rval;
}
