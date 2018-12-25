#ifndef NS_CORE_NET_TCP_H_
#define NS_CORE_NET_TCP_H_

#include "core/core-config.h"
#include "core/common/encoding.h"

typedef struct _ip6_tcp_header {
    uint16_t source;
    uint16_t destination;
    uint32_t sequence_number;
    uint32_t ack_number;
    uint16_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_pointer;
} ip6_tcp_header_t;

uint16_t
ip6_tcp_header_get_source_port(ip6_tcp_header_t *ip6_tcp_header);

uint16_t
ip6_tcp_header_get_destination_port(ip6_tcp_header_t *ip6_tcp_header);

uint32_t
ip6_tcp_header_get_sequence_number(ip6_tcp_header_t *ip6_tcp_header);

uint32_t
ip6_tcp_header_get_acknowledgment_number(ip6_tcp_header_t *ip6_tcp_header);

uint16_t
ip6_tcp_header_get_window(ip6_tcp_header_t *ip6_tcp_header);

uint16_t
ip6_tcp_header_get_checksum(ip6_tcp_header_t *ip6_tcp_header);

uint16_t
ip6_tcp_header_get_urgent_pointer(ip6_tcp_header_t *ip6_tcp_header);

#endif // NS_CORE_NET_TCP_H_
