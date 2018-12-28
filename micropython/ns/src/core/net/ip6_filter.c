#include "core/net/ip6_filter.h"
#include "core/common/code_utils.h"
#include "core/common/instance.h"
#include "core/net/ip6.h"
#include "core/net/tcp.h"
#include "core/net/udp6.h"
#include "core/thread/mle.h"

void
ip6_filter_ctor(ip6_filter_t *ip6_filter)
{
    memset(ip6_filter->unsecure_ports, 0, sizeof(ip6_filter->unsecure_ports));
}

bool
ip6_filter_accept(ip6_filter_t *ip6_filter, message_t message)
{
    bool rval = false;
    ip6_header_t ip6;
    ip6_udp_header_t udp;
    ip6_tcp_header_t tcp;
    uint16_t dst_port;

    // allow all received IPv6 datagrams with link security enabled
    if (message_is_link_security_enabled(message)) {
        EXIT_NOW(rval = true);
    }

    // read IPv6 header
    VERIFY_OR_EXIT(sizeof(ip6) == message_read(message, 0, &ip6, sizeof(ip6)));

    // allow only link-local unicast or multicast
    VERIFY_OR_EXIT(ip6_addr_is_link_local(ip6_header_get_destination(&ip6)) ||
                   ip6_addr_is_link_local_multicast(ip6_header_get_destination(&ip6)));

    switch (ip6_header_get_next_header(&ip6)) {
    case IP6_IP_PROTO_UDP:
        // read the UDP header and get the dst port
        VERIFY_OR_EXIT(sizeof(udp) == message_read(message, sizeof(ip6), &udp, sizeof(udp)));
        dst_port = ip6_udp_header_get_destination_port(&udp);
        // allow MLE traffic
        if (dst_port == MLE_UDP_PORT) {
            EXIT_NOW(rval = true);
        }
        break;
    case IP6_IP_PROTO_TCP:
        // read the TCP header and get the dst port
        VERIFY_OR_EXIT(sizeof(tcp) == message_read(message, sizeof(ip6), &tcp, sizeof(tcp)));
        dst_port = ip6_tcp_header_get_destination_port(&tcp);
        break;
    default:
        // allow UDP and TCP traffic only
        EXIT_NOW();
    }

    // check against allowed unsecure port list
    for (int i = 0; i < IP6_FILTER_MAX_UNSECURE_PORTS; i++) {
        if (ip6_filter->unsecure_ports[i] != 0 && ip6_filter->unsecure_ports[i] == dst_port) {
            EXIT_NOW(rval = true);
        }
    }

exit:
    return rval;
}

ns_error_t
ip6_filter_add_unsecure_port(ip6_filter_t *ip6_filter, uint16_t port)
{
    ns_error_t error = NS_ERROR_NONE;

    for (int i = 0; i < IP6_FILTER_MAX_UNSECURE_PORTS; i++) {
        if (ip6_filter->unsecure_ports[i] == port) {
            EXIT_NOW();
        }
    }

    for (int i = 0; i < IP6_FILTER_MAX_UNSECURE_PORTS; i++) {
        if (ip6_filter->unsecure_ports[i] == 0) {
            ip6_filter->unsecure_ports[i] = port;
            EXIT_NOW();
        }
    }

    EXIT_NOW(error = NS_ERROR_NO_BUFS);

exit:
    return error;
}

ns_error_t
ip6_filter_remove_unsecure_port(ip6_filter_t *ip6_filter, uint16_t port)
{
    ns_error_t error = NS_ERROR_NONE;

    for (int i = 0; i < IP6_FILTER_MAX_UNSECURE_PORTS; i++) {
        if (ip6_filter->unsecure_ports[i] == port) {
            // shift all of the ports higher than this port down
            for (; i < IP6_FILTER_MAX_UNSECURE_PORTS - 1; i++) {
                ip6_filter->unsecure_ports[i] = ip6_filter->unsecure_ports[i + 1];
            }
            // clear the last port entry
            ip6_filter->unsecure_ports[i] = 0;
            EXIT_NOW();
        }
    }

    EXIT_NOW(error = NS_ERROR_NOT_FOUND);

exit:
    return error;
}

void
ip6_filter_remove_all_unsecure_ports(ip6_filter_t *ip6_filter)
{
    memset(ip6_filter->unsecure_ports, 0, sizeof(ip6_filter->unsecure_ports));
}

const uint16_t *
ip6_filter_get_unsecure_ports(ip6_filter_t *ip6_filter, uint8_t *num_entries)
{
    // count the number of unsecure ports
    for (uint8_t i = 0; i < IP6_FILTER_MAX_UNSECURE_PORTS; i++) {
        if (ip6_filter->unsecure_ports[i] == 0) {
            *num_entries = i;
            break;
        }
    }

    return ip6_filter->unsecure_ports;
}
