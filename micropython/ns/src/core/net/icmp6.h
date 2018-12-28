#ifndef NS_CORE_NET_ICMP6_H_
#define NS_CORE_NET_ICMP6_H_

#include "core/core-config.h"
#include "ns/icmp6.h"
#include "core/common/encoding.h"
#include "core/net/ip6_headers.h"

typedef ns_icmp6_header_t ip6_icmp_header_t;
typedef ns_icmp6_handler_t ip6_icmp_handler_t;

typedef enum _ip6_icmp_header_type {
    IP6_ICMP_HEADER_TYPE_DST_UNREACH  = NS_ICMP6_TYPE_DST_UNREACH,
    IP6_ICMP_HEADER_TYPE_ECHO_REQUEST = NS_ICMP6_TYPE_ECHO_REQUEST,
    IP6_ICMP_HEADER_TYPE_ECHO_REPLY   = NS_ICMP6_TYPE_ECHO_REPLY,
} ip6_icmp_header_type_t;

typedef enum _ip6_icmp_header_code {
    IP6_ICMP_HEADER_CODE_DST_UNREACH_NO_ROUTE = NS_ICMP6_CODE_DST_UNREACH_NO_ROUTE,
} ip6_icmp_header_code_t;

typedef struct _ip6_icmp ip6_icmp_t;
struct _ip6_icmp {
    void *instance;
    ip6_icmp_handler_t *handlers;
    uint16_t echo_sequence;
    ns_icmp6_echo_mode_t echo_mode;
};

// --- icmp header functions
void
ip6_icmp_header_init(ip6_icmp_header_t *ip6_icmp_header);

ip6_icmp_header_type_t
ip6_icmp_header_get_type(ip6_icmp_header_t *ip6_icmp_header);

void
ip6_icmp_header_set_type(ip6_icmp_header_t *ip6_icmp_header, ip6_icmp_header_type_t type);

ip6_icmp_header_code_t
ip6_icmp_header_get_code(ip6_icmp_header_t *ip6_icmp_header);

void
ip6_icmp_header_set_code(ip6_icmp_header_t *ip6_icmp_header, ip6_icmp_header_code_t code);

uint16_t
ip6_icmp_header_get_checksum(ip6_icmp_header_t *ip6_icmp_header);

void
ip6_icmp_header_set_checksum(ip6_icmp_header_t *ip6_icmp_header, uint16_t checksum);

uint16_t
ip6_icmp_header_get_id(ip6_icmp_header_t *ip6_icmp_header);

void
ip6_icmp_header_set_id(ip6_icmp_header_t *ip6_icmp_header, uint16_t id);

uint16_t
ip6_icmp_header_get_sequence(ip6_icmp_header_t *ip6_icmp_header);

void
ip6_icmp_header_set_sequence(ip6_icmp_header_t *ip6_icmp_header, uint16_t sequence);

uint8_t
ip6_icmp_header_get_checksum_offset(void);

uint8_t
ip6_icmp_header_get_data_offset(void);

// --- icmp handler functions
void
ip6_icmp_handler_ctor(ip6_icmp_handler_t *ip6_icmp_handler,
                      ns_icmp6_receive_callback_func_t callback,
                      void *context);

// --- icmp functions
void
ip6_icmp_ctor(ip6_icmp_t *ip6_icmp, void *instance);

message_t
ip6_icmp_new_message(ip6_icmp_t *ip6_icmp, uint16_t reserved);

ns_error_t
ip6_icmp_register_handler(ip6_icmp_t *ip6_icmp, ip6_icmp_handler_t *handler);

ns_error_t
ip6_icmp_send_echo_request(ip6_icmp_t *ip6_icmp,
                           message_t message,
                           const ip6_message_info_t *message_info,
                           uint16_t identifier);

ns_error_t
ip6_icmp_send_error(ip6_icmp_t *ip6_icmp,
                    ip6_icmp_header_type_t type,
                    ip6_icmp_header_code_t code,
                    const ip6_message_info_t *message_info,
                    const ip6_header_t *header);

ns_error_t
ip6_icmp_handle_message(ip6_icmp_t *ip6_icmp,
                        message_t message,
                        ip6_message_info_t *message_info);

ns_error_t
ip6_icmp_update_checksum(ip6_icmp_t *ip6_icmp,
                         message_t message,
                         uint16_t checksum);

ns_icmp6_echo_mode_t
ip6_icmp_get_echo_mode(ip6_icmp_t *ip6_icmp);

void
ip6_icmp_set_echo_mode(ip6_icmp_t *ip6_icmp, ns_icmp6_echo_mode_t mode);

bool
ip6_icmp_should_handle_echo_request(ip6_icmp_t *ip6_icmp,
                                    const ip6_message_info_t *message_info);

#endif // NS_CORE_NET_ICMP6_H_
