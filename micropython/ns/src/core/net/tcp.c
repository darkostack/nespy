#include "core/net/tcp.h"

uint16_t
ip6_tcp_header_get_source_port(ip6_tcp_header_t *ip6_tcp_header)
{
    return encoding_swap16(ip6_tcp_header->source);
}

uint16_t
ip6_tcp_header_get_destination_port(ip6_tcp_header_t *ip6_tcp_header)
{
    return encoding_swap16(ip6_tcp_header->destination);
}

uint32_t
ip6_tcp_header_get_sequence_number(ip6_tcp_header_t *ip6_tcp_header)
{
    return encoding_swap32(ip6_tcp_header->sequence_number);
}

uint32_t
ip6_tcp_header_get_acknowledgment_number(ip6_tcp_header_t *ip6_tcp_header)
{
    return encoding_swap32(ip6_tcp_header->ack_number);
}

uint16_t
ip6_tcp_header_get_window(ip6_tcp_header_t *ip6_tcp_header)
{
    return encoding_swap16(ip6_tcp_header->window);
}

uint16_t
ip6_tcp_header_get_checksum(ip6_tcp_header_t *ip6_tcp_header)
{
    return encoding_swap16(ip6_tcp_header->checksum);
}

uint16_t
ip6_tcp_header_get_urgent_pointer(ip6_tcp_header_t *ip6_tcp_header)
{
    return encoding_swap16(ip6_tcp_header->urgent_pointer);
}
