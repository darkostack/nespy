#ifndef NS_CORE_NET_ICMP6_H_
#define NS_CORE_NET_ICMP6_H_

#include "core/core-config.h"
#include "ns/icmp6.h"
#include "core/common/encoding.h"
#include "core/net/ip6_headers.h"

typedef ns_icmp6_header_t ip6_icmp6_header_t;

typedef enum _ip6_icmp6_header_type {
    IP6_ICMP6_HEADER_TYPE_DST_UNREACH  = NS_ICMP6_TYPE_DST_UNREACH,
    IP6_ICMP6_HEADER_TYPE_ECHO_REQUEST = NS_ICMP6_TYPE_ECHO_REQUEST,
    IP6_ICMP6_HEADER_TYPE_ECHO_REPLY   = NS_ICMP6_TYPE_ECHO_REPLY,
} ip6_icmp6_header_type_t;

typedef enum _ip6_icmp6_header_code {
    IP6_ICMP6_HEADER_CODE_DST_UNREACH_NO_ROUTE = NS_ICMP6_CODE_DST_UNREACH_NO_ROUTE,
} ip6_icmp6_header_code_t;

// --- icmp6 header functions
void
ip6_icmp6_header_init(ip6_icmp6_header_t *ip6_icmp6_header);

ip6_icmp6_header_type_t
ip6_icmp6_header_get_type(ip6_icmp6_header_t *ip6_icmp6_header);

void
ip6_icmp6_header_set_type(ip6_icmp6_header_t *ip6_icmp6_header, ip6_icmp6_header_type_t type);

ip6_icmp6_header_code_t
ip6_icmp6_header_get_code(ip6_icmp6_header_t *ip6_icmp6_header);

void
ip6_icmp6_header_set_code(ip6_icmp6_header_t *ip6_icmp6_header, ip6_icmp6_header_code_t code);

uint16_t
ip6_icmp6_header_get_checksum(ip6_icmp6_header_t *ip6_icmp6_header);

void
ip6_icmp6_header_set_checksum(ip6_icmp6_header_t *ip6_icmp6_header, uint16_t checksum);

uint16_t
ip6_icmp6_header_get_id(ip6_icmp6_header_t *ip6_icmp6_header);

void
ip6_icmp6_header_set_id(ip6_icmp6_header_t *ip6_icmp6_header, uint16_t id);

uint16_t
ip6_icmp6_header_get_sequence(ip6_icmp6_header_t *ip6_icmp6_header);

void
ip6_icmp6_header_set_sequence(ip6_icmp6_header_t *ip6_icmp6_header, uint16_t sequence);

uint8_t
ip6_icmp6_header_get_checksum_offset(ip6_icmp6_header_t *ip6_icmp6_header);

uint8_t
ip6_icmp6_header_get_data_offset(ip6_icmp6_header_t *ip6_icmp6_header);

#endif // NS_CORE_NET_ICMP6_H_
