#ifndef NS_ICMP6_H_
#define NS_ICMP6_H_

#include "ns/ip6.h"
#include "ns/message.h"

typedef enum _ns_icmp6_type {
    NS_ICMP6_TYPE_DST_UNREACH  = 1,
    NS_ICMP6_TYPE_ECHO_REQUEST = 128,
    NS_ICMP6_TYPE_ECHO_REPLY   = 129,
} ns_icmp6_type_t;

typedef enum _ns_icmp6_code {
    NS_ICMP6_CODE_DST_UNREACH_NO_ROUTE = 0,
} ns_icmp6_code_t;

#define NS_ICMP6_HEADER_DATA_SIZE 4 // size of an message specific data of ICMPv6 header

typedef struct _ns_icmp6_header ns_icmp6_header_t;
struct _ns_icmp6_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union {
        uint8_t m8[NS_ICMP6_HEADER_DATA_SIZE / sizeof(uint8_t)];
        uint16_t m16[NS_ICMP6_HEADER_DATA_SIZE / sizeof(uint16_t)];
        uint32_t m32[NS_ICMP6_HEADER_DATA_SIZE / sizeof(uint32_t)];
    } data;
};

typedef void (*ns_icmp6_receive_callback_func_t)(void *context,
                                                 ns_message_t message,
                                                 const ns_message_info_t *message_info,
                                                 const ns_icmp6_header_t *icmp_header);

typedef struct _ns_icmp6_handler ns_icmp6_handler_t;
struct _ns_icmp6_handler {
    ns_icmp6_receive_callback_func_t receive_callback;
    void *context;
    ns_icmp6_handler_t *next;
};

typedef enum _ns_icmp6_echo_mode {
    NS_ICMP6_ECHO_HANDLER_DISABLED       = 0, // ICMPv6 Echo processing disabled
    NS_ICMP6_ECHO_HANDLER_UNICAST_ONLY   = 1, // ICMPv6 Echo processing enabled only for unicast requests only
    NS_ICMP6_ECHO_HANDLER_MULTICAST_ONLY = 2, // ICMPv6 Echo processing enabled only for multicast requests only
    NS_ICMP6_ECHO_HANDLER_ALL            = 3, // ICMPv6 Echo processing enabled for unicast and multicast requests
} ns_icmp6_echo_mode_t;

ns_icmp6_echo_mode_t
ns_icmp6_get_echo_mode(ns_instance_t instance);

void
ns_icmp6_set_echo_mode(ns_instance_t instance, ns_icmp6_echo_mode_t mode);

ns_error_t
ns_icmp6_register_handler(ns_instance_t instance, ns_icmp6_handler_t *handler);

ns_error_t
ns_icmp6_send_echo_request(ns_instance_t instance,
                           ns_message_t message,
                           const ns_message_info_t *message_info,
                           uint16_t indetifier);

#endif // NS_ICMP6_H_
