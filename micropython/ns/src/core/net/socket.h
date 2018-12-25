#ifndef NS_CORE_NET_SOCKET_H_
#define NS_CORE_NET_SOCKET_H_

#include "core/core-config.h"
#include "core/net/ip6_address.h"

typedef ns_message_info_t ip6_message_info_t;

void
ip6_message_info_ctor(ip6_message_info_t *ip6_message_info);

const ip6_addr_t
ip6_message_info_get_sock_addr(ip6_message_info_t *ip6_message_info);

void
ip6_message_info_set_sock_addr(ip6_message_info_t *ip6_message_info, ip6_addr_t ip6_addr);

uint16_t
ip6_message_info_get_sock_port(ip6_message_info_t *ip6_message_info);

void
ip6_message_info_set_sock_port(ip6_message_info_t *ip6_message_info, uint16_t port);

ip6_addr_t
ip6_message_info_get_peer_addr(ip6_message_info_t *ip6_message_info);

void
ip6_message_info_set_peer_addr(ip6_message_info_t *ip6_message_info, ip6_addr_t ip6_addr);

uint16_t
ip6_message_info_get_peer_port(ip6_message_info_t *ip6_message_info);

void
ip6_message_info_set_peer_port(ip6_message_info_t *ip6_message_info, uint16_t port);

int8_t
ip6_message_info_get_interface_id(ip6_message_info_t *ip6_message_info);

void
ip6_message_info_set_interface_id(ip6_message_info_t *ip6_message_info, int8_t interface_id);

uint8_t
ip6_message_info_get_hop_limit(ip6_message_info_t *ip6_message_info);

void
ip6_message_info_set_hop_limit(ip6_message_info_t *ip6_message_info, uint8_t hop_limit);

const void *
ip6_message_info_get_link_info(ip6_message_info_t *ip6_message_info);

void
ip6_message_info_set_link_info(ip6_message_info_t *ip6_message_info, const void *link_info);

#endif // NS_CORE_NET_SOCKET_H_
