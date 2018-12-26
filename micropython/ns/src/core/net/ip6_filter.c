#include "core/net/ip6_filter.h"
#include "core/common/code_utils.h"
#include "core/common/instance.h"
#include "core/net/ip6.h"
#include "core/net/tcp.h"
//#include "core/net/udp6.h"
//#include "core/thread/mle.h"

void
ip6_filter_ctor(ip6_filter_t *ip6_filter)
{
    memset(ip6_filter->unsecure_ports, 0, sizeof(ip6_filter->unsecure_ports));
}

bool
ip6_filter_accept(ip6_filter_t *ip6_filter, message_t message)
{
    // TODO:
    return false;
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
