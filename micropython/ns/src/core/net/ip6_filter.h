#ifndef NS_CORE_NET_IP6_FILTER_H_
#define NS_CORE_NET_IP6_FILTER_H_

#include "core/core-config.h"
#include "core/common/message.h"

enum {
    IP6_FILTER_MAX_UNSECURE_PORTS = 2,
};

typedef struct _ip6_filter {
    uint16_t unsecure_ports[IP6_FILTER_MAX_UNSECURE_PORTS];
} ip6_filter_t;

void
ip6_filter_ctor(ip6_filter_t *ip6_filter);

bool
ip6_filter_accept(ip6_filter_t *ip6_filter, message_t message);

ns_error_t
ip6_filter_add_unsecure_port(ip6_filter_t *ip6_filter, uint16_t port);

ns_error_t
ip6_filter_remove_unsecure_port(ip6_filter_t *ip6_filter, uint16_t port);

void
ip6_filter_remove_all_unsecure_ports(ip6_filter_t *ip6_filter);

const uint16_t *
ip6_filter_get_unsecure_ports(ip6_filter_t *ip6_filter, uint8_t *num_entries);

#endif // NS_CORE_NET_IP6_FILTER_H_
