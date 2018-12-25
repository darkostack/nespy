#ifndef NS_CORE_NET_IP6_HEADERS_H_
#define NS_CORE_NET_IP6_HEADERS_H_

#include "core/core-config.h"
#include "core/common/encoding.h"
#include "core/common/message.h"
#include "core/net/ip6_address.h"
#include "core/net/socket.h"

typedef enum _ip6_ip_proto {
    IP6_IP_PROTO_HOP_OPTS = 0,
    IP6_IP_PROTO_TCP      = 6,
    IP6_IP_PROTO_UDP      = 17,
    IP6_IP_PROTO_IP6      = 41,
    IP6_IP_PROTO_ROUTING  = 43,
    IP6_IP_PROTO_FRAGMENT = 44,
    IP6_IP_PROTO_ICMP6    = 58,
    IP6_IP_PROTO_NONE     = 59,
    IP6_IP_PROTO_DST_OPTS = 60,
} ip6_ip_proto_t;

typedef enum _ip6_ip_dscp_cs {
    IP6_DSCP_CS0    = 0,    // Class selector codepoint 0
    IP6_DSCP_CS1    = 8,    // Class selector codepoint 8
    IP6_DSCP_CS2    = 16,   // Class selector codepoint 16
    IP6_DSCP_CS3    = 24,   // Class selector codepoint 24
    IP6_DSCP_CS4    = 32,   // Class selector codepoint 32
    IP6_DSCP_CS5    = 40,   // Class selector codepoint 40
    IP6_DSCP_CS6    = 48,   // Class selector codepoint 48
    IP6_DSCP_CS7    = 56,   // Class selector codepoint 56
    IP6_DSCP_CSMASK = 0x38, // Class selector mask
} ip6_ip_dscp_cs_t;

enum {
    IP6_VERSION_CLASS_FLOW_SIZE = 4,
};

enum {
    IP6_HEADER_VERSION6     = 0x60,
    IP6_HEADER_VERSION_MASK = 0xf0,
    IP6_HEADER_DSCP_OFFSET  = 22,
    IP6_HEADER_DSCP_MASK    = 0xfc00000,
};

typedef struct _ip6_header ip6_header_t;
struct _ip6_header {
    union {
        uint8_t m8[IP6_VERSION_CLASS_FLOW_SIZE / sizeof(uint8_t)];
        uint16_t m16[IP6_VERSION_CLASS_FLOW_SIZE / sizeof(uint16_t)];
        uint32_t m32[IP6_VERSION_CLASS_FLOW_SIZE / sizeof(uint32_t)];
    } version_class_flow;
    uint16_t payload_length;
    uint8_t next_header;
    uint8_t hop_limit;
    ns_ip6_addr_t source;
    ns_ip6_addr_t destination;
};

typedef struct _ip6_extension_header {
    uint8_t next_header;
    uint8_t length;
} ip6_extension_header_t;

typedef ip6_extension_header_t ip6_hop_by_hop_header_t;

typedef enum _ip6_option_header_action {
    IP6_OPTION_HEADER_ACTION_SKIP       = 0x00,
    IP6_OPTION_HEADER_ACTION_DISCARD    = 0x40,
    IP6_OPTION_HEADER_ACTION_FORCE_ICMP = 0x80,
    IP6_OPTION_HEADER_ACTION_ICMP       = 0xc0,
    IP6_OPTION_HEADER_ACTION_MASK       = 0xc0,
} ip6_option_header_action_t;

typedef struct _ip6_option_header {
    uint8_t type;
    uint8_t length;
} ip6_option_header_t;

enum {
    IP6_OPTION_PADN_TYPE = 0x01,
    IP6_OPTION_PADN_DATA = 0x00,
    IP6_OPTION_PADN_MAX_LENGTH = 0x05,
};

typedef struct _ip6_option_padn {
    ip6_option_header_t option_header;
    uint8_t pad[IP6_OPTION_PADN_MAX_LENGTH];
} ip6_option_padn_t;

enum {
    IP6_OPTION_PAD1_TYPE = 0x00,
};

typedef struct _ip6_option_pad1 {
    uint8_t type;
} ip6_option_pad1_t;

enum {
    IP6_FRAGMENT_HEADER_OFFSET_OFFSET = 3,
    IP6_FRAGMENT_HEADER_OFFSET_MASK = 0xfff8,
    IP6_FRAGMENT_HEADER_MORE_FLAG = 1,
};

typedef struct _ip6_fragment_header {
    uint8_t next_header;
    uint8_t reserved;
    uint16_t offset_more;
    uint32_t identification;
} ip6_fragment_header_t;

// --- ip6 header functions
void
ip6_header_init(ip6_header_t *ip6_header);

void
ip6_header_init_set_version_class_flow(ip6_header_t *ip6_header, uint32_t version_class_flow);

ns_error_t
ip6_header_init_from_message(ip6_header_t *ip6_header, const message_t message);

bool
ip6_header_is_version6(ip6_header_t *ip6_header);

uint8_t
ip6_header_get_dscp(ip6_header_t *ip6_header);

void
ip6_header_set_dscp(ip6_header_t *ip6_header, uint8_t dscp);

uint16_t
ip6_header_get_payload_length(ip6_header_t *ip6_header);

void
ip6_header_set_payload_length(ip6_header_t *ip6_header, uint16_t length);

ip6_ip_proto_t
ip6_header_get_next_header(ip6_header_t *ip6_header);

void
ip6_header_set_next_header(ip6_header_t *ip6_header, ip6_ip_proto_t next_header);

uint8_t
ip6_header_get_hop_limit(ip6_header_t *ip6_header);

void
ip6_header_set_hop_limit(ip6_header_t *ip6_header, uint8_t hop_limit);

ip6_addr_t *
ip6_header_get_source(ip6_header_t *ip6_header);

void
ip6_header_set_source(ip6_header_t *ip6_header, const ip6_addr_t *source);

ip6_addr_t *
ip6_header_get_destination(ip6_header_t *ip6_header);

void
ip6_header_set_destination(ip6_header_t *ip6_header, const ip6_addr_t *destination);

uint8_t
ip6_header_get_payload_length_offset(ip6_header_t *ip6_header);

uint8_t
ip6_header_get_hop_limit_offset(ip6_header_t *ip6_header);

uint8_t
ip6_header_get_hop_limit_size(ip6_header_t *ip6_header);

uint8_t
ip6_header_get_destination_offset(ip6_header_t *ip6_header);

// --- ip6 extension header functions
ip6_ip_proto_t
ip6_extension_header_get_next_header(ip6_extension_header_t *ip6_extension_header);

void
ip6_extension_header_set_next_header(ip6_extension_header_t *ip6_extension_header, ip6_ip_proto_t next_header);

uint8_t
ip6_extension_header_get_length(ip6_extension_header_t *ip6_extension_header);

void
ip6_extension_header_set_length(ip6_extension_header_t *ip6_extension_header, uint8_t length);

// --- ip6 option header functions
void
ip6_option_header_ctor(ip6_option_header_t *ip6_option_header);

uint8_t
ip6_option_header_get_type(ip6_option_header_t *ip6_option_header);

void
ip6_option_header_set_type(ip6_option_header_t *ip6_option_header, uint8_t type);

ip6_option_header_action_t
ip6_option_header_get_action(ip6_option_header_t *ip6_option_header);

uint8_t
ip6_option_header_get_length(ip6_option_header_t *ip6_option_header);

void
ip6_option_header_set_length(ip6_option_header_t *ip6_option_header, uint8_t length);

// --- ip6 option padn functions
void
ip6_option_padn_init(ip6_option_padn_t *ip6_option_padn, uint8_t pad_length);

uint8_t
ip6_option_padn_get_total_length(ip6_option_padn_t *ip6_option_padn);

// --- ip6 option pad1
void
ip6_option_pad1_init(ip6_option_pad1_t *ip6_option_pad1);

// --- ip6 fragment header
void
ip6_fragment_header_init(ip6_fragment_header_t *ip6_fragment_header);

ip6_ip_proto_t
ip6_fragment_header_get_next_header(ip6_fragment_header_t *ip6_fragment_header);

void
ip6_fragment_header_set_next_header(ip6_fragment_header_t *ip6_fragment_header, ip6_ip_proto_t next_header);

uint16_t
ip6_fragment_header_get_offset(ip6_fragment_header_t *ip6_fragment_header);

void
ip6_fragment_header_set_offset(ip6_fragment_header_t *ip6_fragment_header, uint16_t offset);

bool
ip6_fragment_header_is_more_flag_set(ip6_fragment_header_t *ip6_fragment_header);

void
ip6_fragment_header_clear_more_flag(ip6_fragment_header_t *ip6_fragment_header);

void
ip6_fragment_header_set_more_flag(ip6_fragment_header_t *ip6_fragment_header);

#endif // NS_CORE_NET_IP6_HEADERS_H_
