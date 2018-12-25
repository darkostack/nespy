#include "core/net/socket.h"

void
ip6_message_info_ctor(ip6_message_info_t *ip6_message_info)
{
    memset(ip6_message_info, 0, sizeof(*ip6_message_info));
}

const ip6_addr_t
ip6_message_info_get_sock_addr(ip6_message_info_t *ip6_message_info)
{
    return (ip6_addr_t)ip6_message_info->sock_addr;
}

void
ip6_message_info_set_sock_addr(ip6_message_info_t *ip6_message_info, ip6_addr_t ip6_addr)
{
    ip6_message_info->sock_addr = ip6_addr;
}

uint16_t
ip6_message_info_get_sock_port(ip6_message_info_t *ip6_message_info)
{
    return ip6_message_info->sock_port;
}

void
ip6_message_info_set_sock_port(ip6_message_info_t *ip6_message_info, uint16_t port)
{
    ip6_message_info->sock_port = port;
}

ip6_addr_t
ip6_message_info_get_peer_addr(ip6_message_info_t *ip6_message_info)
{
    return (ip6_addr_t)ip6_message_info->peer_addr;
}

void
ip6_message_info_set_peer_addr(ip6_message_info_t *ip6_message_info, ip6_addr_t ip6_addr)
{
    ip6_message_info->peer_addr = ip6_addr;
}

uint16_t
ip6_message_info_get_peer_port(ip6_message_info_t *ip6_message_info)
{
    return ip6_message_info->peer_port;
}

void
ip6_message_info_set_peer_port(ip6_message_info_t *ip6_message_info, uint16_t port)
{
    ip6_message_info->peer_port = port;
}

int8_t
ip6_message_info_get_interface_id(ip6_message_info_t *ip6_message_info)
{
    return ip6_message_info->interface_id;
}

void
ip6_message_info_set_interface_id(ip6_message_info_t *ip6_message_info, int8_t interface_id)
{
    ip6_message_info->interface_id = interface_id;
}

uint8_t
ip6_message_info_get_hop_limit(ip6_message_info_t *ip6_message_info)
{
    return ip6_message_info->hop_limit;
}

void
ip6_message_info_set_hop_limit(ip6_message_info_t *ip6_message_info, uint8_t hop_limit)
{
    ip6_message_info->hop_limit = hop_limit;
}

const void *
ip6_message_info_get_link_info(ip6_message_info_t *ip6_message_info)
{
    return ip6_message_info->link_info;
}

void
ip6_message_info_set_link_info(ip6_message_info_t *ip6_message_info, const void *link_info)
{
    ip6_message_info->link_info = link_info;
}
