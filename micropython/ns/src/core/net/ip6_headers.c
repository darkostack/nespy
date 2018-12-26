#include "core/common/code_utils.h"
#include "core/net/ip6_headers.h"
#include "core/net/ip6.h"
#include <stddef.h>

// --- ip6 header functions
void
ip6_header_init(ip6_header_t *ip6_header)
{
    ip6_header->version_class_flow.m32[0] = 0;
    ip6_header->version_class_flow.m8[0] = IP6_HEADER_VERSION6;
}

void
ip6_header_init_set_version_class_flow(ip6_header_t *ip6_header, uint32_t version_class_flow)
{
    ip6_header->version_class_flow.m32[0] = encoding_big_endian_swap32(version_class_flow);
}

ns_error_t
ip6_header_init_from_message(ip6_header_t *ip6_header, const message_t message)
{
    ns_error_t error = NS_ERROR_NONE;
    uint16_t length;

    // check message length
    VERIFY_OR_EXIT(message_read(message, 0, ip6_header, sizeof(*ip6_header)), error = NS_ERROR_PARSE);

    // check version
    VERIFY_OR_EXIT(ip6_header_is_version6(ip6_header), error = NS_ERROR_PARSE);

    // check payload length
    length = sizeof(*ip6_header) + ip6_header_get_payload_length(ip6_header);
    VERIFY_OR_EXIT(length == message_get_length(message) && length <= IP6_MAX_DATAGRAM_LENGTH,
                   error = NS_ERROR_PARSE);

exit:
    return error;
}

bool
ip6_header_is_version6(ip6_header_t *ip6_header)
{
    return (ip6_header->version_class_flow.m8[0] & IP6_HEADER_VERSION_MASK) == IP6_HEADER_VERSION6;
}

uint8_t
ip6_header_get_dscp(ip6_header_t *ip6_header)
{
    return (uint8_t)((encoding_big_endian_swap32(ip6_header->version_class_flow.m32[0]) & IP6_HEADER_DSCP_MASK) >> IP6_HEADER_DSCP_OFFSET);
}

void
ip6_header_set_dscp(ip6_header_t *ip6_header, uint8_t dscp)
{
    uint32_t tmp = encoding_big_endian_swap32(ip6_header->version_class_flow.m32[0]);
    tmp = (tmp & (uint32_t)(~IP6_HEADER_DSCP_MASK)) | (((uint32_t)(dscp) << IP6_HEADER_DSCP_OFFSET) & IP6_HEADER_DSCP_MASK);
    ip6_header->version_class_flow.m32[0] = encoding_big_endian_swap32(tmp);
}

uint16_t
ip6_header_get_payload_length(ip6_header_t *ip6_header)
{
    return encoding_big_endian_swap16(ip6_header->payload_length);
}

void
ip6_header_set_payload_length(ip6_header_t *ip6_header, uint16_t length)
{
    ip6_header->payload_length = encoding_big_endian_swap16(length);
}

ip6_ip_proto_t
ip6_header_get_next_header(ip6_header_t *ip6_header)
{
    return (ip6_ip_proto_t)ip6_header->next_header;
}

void
ip6_header_set_next_header(ip6_header_t *ip6_header, ip6_ip_proto_t next_header)
{
    ip6_header->next_header = (uint8_t)next_header;
}

uint8_t
ip6_header_get_hop_limit(ip6_header_t *ip6_header)
{
    return ip6_header->hop_limit;
}

void
ip6_header_set_hop_limit(ip6_header_t *ip6_header, uint8_t hop_limit)
{
    ip6_header->hop_limit = hop_limit;
}

ip6_addr_t *
ip6_header_get_source(ip6_header_t *ip6_header)
{
    return (ip6_addr_t *)&ip6_header->source;
}

void
ip6_header_set_source(ip6_header_t *ip6_header, const ip6_addr_t *source)
{
    ip6_header->source = *source;
}

ip6_addr_t *
ip6_header_get_destination(ip6_header_t *ip6_header)
{
    return (ip6_addr_t *)&ip6_header->destination;
}

void
ip6_header_set_destination(ip6_header_t *ip6_header, const ip6_addr_t *destination)
{
    ip6_header->destination = *destination;
}

uint8_t
ip6_header_get_payload_length_offset(ip6_header_t *ip6_header)
{
    (void)ip6_header;
    return offsetof(ip6_header_t, payload_length);
}

uint8_t
ip6_header_get_hop_limit_offset(ip6_header_t *ip6_header)
{
    (void)ip6_header;
    return offsetof(ip6_header_t, hop_limit);
}

uint8_t
ip6_header_get_hop_limit_size(ip6_header_t *ip6_header)
{
    (void)ip6_header;
    return sizeof(uint8_t);
}

uint8_t
ip6_header_get_destination_offset(ip6_header_t *ip6_header)
{
    (void)ip6_header;
    return offsetof(ip6_header_t, destination);
}

// --- ip6 extension header functions
ip6_ip_proto_t
ip6_extension_header_get_next_header(ip6_extension_header_t *ip6_extension_header)
{
    return (ip6_ip_proto_t)ip6_extension_header->next_header;
}

void
ip6_extension_header_set_next_header(ip6_extension_header_t *ip6_extension_header, ip6_ip_proto_t next_header)
{
    ip6_extension_header->next_header = (uint8_t)next_header;
}

uint8_t
ip6_extension_header_get_length(ip6_extension_header_t *ip6_extension_header)
{
    return ip6_extension_header->length;
}

void
ip6_extension_header_set_length(ip6_extension_header_t *ip6_extension_header, uint8_t length)
{
    ip6_extension_header->length = length;
}

// --- ip6 hop by hop header functions
ip6_ip_proto_t
ip6_hop_by_hop_header_get_next_header(ip6_hop_by_hop_header_t *ip6_hop_by_hop_header)
{
    return (ip6_ip_proto_t)ip6_hop_by_hop_header->next_header;
}

void
ip6_hop_by_hop_header_set_next_header(ip6_hop_by_hop_header_t *ip6_hop_by_hop_header, ip6_ip_proto_t next_header)
{
    ip6_hop_by_hop_header->next_header = (uint8_t)next_header;
}

uint8_t
ip6_hop_by_hop_header_get_length(ip6_hop_by_hop_header_t *ip6_hop_by_hop_header)
{
    return ip6_hop_by_hop_header->length;
}

void
ip6_hop_by_hop_header_set_length(ip6_hop_by_hop_header_t *ip6_hop_by_hop_header, uint8_t length)
{
    ip6_hop_by_hop_header->length = length;
}

// --- ip6 option header functions
void
ip6_option_header_ctor(ip6_option_header_t *ip6_option_header)
{
    ip6_option_header->type = 0;
    ip6_option_header->length = 0;
}

uint8_t
ip6_option_header_get_type(ip6_option_header_t *ip6_option_header)
{
    return ip6_option_header->type;
}

void
ip6_option_header_set_type(ip6_option_header_t *ip6_option_header, uint8_t type)
{
    ip6_option_header->type = type;
}

ip6_option_header_action_t
ip6_option_header_get_action(ip6_option_header_t *ip6_option_header)
{
    return (ip6_option_header_action_t)(ip6_option_header->type & IP6_OPTION_HEADER_ACTION_MASK);
}

uint8_t
ip6_option_header_get_length(ip6_option_header_t *ip6_option_header)
{
    return ip6_option_header->length;
}

void
ip6_option_header_set_length(ip6_option_header_t *ip6_option_header, uint8_t length)
{
    ip6_option_header->length = length;
}

// --- ip6 option padn functions
void
ip6_option_padn_init(ip6_option_padn_t *ip6_option_padn, uint8_t pad_length)
{
    ip6_option_header_set_type(&ip6_option_padn->option_header, IP6_OPTION_PADN_TYPE);
    ip6_option_header_set_length(&ip6_option_padn->option_header, pad_length - sizeof(ip6_option_header_t));
    memset(ip6_option_padn->pad, IP6_OPTION_PADN_DATA, pad_length - sizeof(ip6_option_header_t));
}

uint8_t
ip6_option_padn_get_total_length(ip6_option_padn_t *ip6_option_padn)
{
    return ip6_option_header_get_length(&ip6_option_padn->option_header) + sizeof(ip6_option_header_t);
}

// --- ip6 option pad1
void
ip6_option_pad1_init(ip6_option_pad1_t *ip6_option_pad1)
{
    ip6_option_pad1->type = IP6_OPTION_PAD1_TYPE;
}

// --- ip6 fragment header
void
ip6_fragment_header_init(ip6_fragment_header_t *ip6_fragment_header)
{
    ip6_fragment_header->reserved = 0;
    ip6_fragment_header->identification = 0;
}

ip6_ip_proto_t
ip6_fragment_header_get_next_header(ip6_fragment_header_t *ip6_fragment_header)
{
    return (ip6_ip_proto_t)(ip6_fragment_header->next_header);
}

void
ip6_fragment_header_set_next_header(ip6_fragment_header_t *ip6_fragment_header, ip6_ip_proto_t next_header)
{
    ip6_fragment_header->next_header = (uint8_t)next_header;
}

uint16_t
ip6_fragment_header_get_offset(ip6_fragment_header_t *ip6_fragment_header)
{
    return (encoding_big_endian_swap16(ip6_fragment_header->offset_more) & IP6_FRAGMENT_HEADER_OFFSET_MASK) >> IP6_FRAGMENT_HEADER_OFFSET_OFFSET;
}

void
ip6_fragment_header_set_offset(ip6_fragment_header_t *ip6_fragment_header, uint16_t offset)
{
    uint16_t tmp = encoding_big_endian_swap16(ip6_fragment_header->offset_more);
    tmp = (tmp & ~IP6_FRAGMENT_HEADER_OFFSET_MASK) | ((offset << IP6_FRAGMENT_HEADER_OFFSET_OFFSET) & IP6_FRAGMENT_HEADER_OFFSET_MASK);
    ip6_fragment_header->offset_more = encoding_big_endian_swap16(tmp);
}

bool
ip6_fragment_header_is_more_flag_set(ip6_fragment_header_t *ip6_fragment_header)
{
    return encoding_big_endian_swap16(ip6_fragment_header->offset_more) & IP6_FRAGMENT_HEADER_MORE_FLAG;
}

void
ip6_fragment_header_clear_more_flag(ip6_fragment_header_t *ip6_fragment_header)
{
    ip6_fragment_header->offset_more = encoding_big_endian_swap16(encoding_big_endian_swap16(ip6_fragment_header->offset_more) & ~IP6_FRAGMENT_HEADER_MORE_FLAG);
}

void
ip6_fragment_header_set_more_flag(ip6_fragment_header_t *ip6_fragment_header)
{
    ip6_fragment_header->offset_more = encoding_big_endian_swap16(encoding_big_endian_swap16(ip6_fragment_header->offset_more) | IP6_FRAGMENT_HEADER_MORE_FLAG);
}
