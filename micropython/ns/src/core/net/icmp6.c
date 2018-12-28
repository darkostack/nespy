#include "core/net/icmp6.h"
#include "core/common/code_utils.h"
#include "core/common/debug.h"
#include "core/common/instance.h"
#include "core/common/message.h"
#include "core/net/ip6.h"
#include <stddef.h>

// --- private functions declarations
#if 0
static void
ip6_icmp_handler_handle_receive_message(ip6_icmp_handler_t *ip6_icmp_handler,
                                        message_t message,
                                        const ip6_message_info_t *ip6_message_info,
                                        const ip6_icmp_header_t *icmp6_header);
#endif

static ip6_icmp_handler_t *
ip6_icmp_handler_get_next(ip6_icmp_handler_t *ip6_icmp_handler);

#if 0
static ns_error_t
ip6_icmp_handle_echo_request(ip6_icmp_t *ip6_icmp, message_t message);
#endif

// --- icmp header functions
void
ip6_icmp_header_init(ip6_icmp_header_t *ip6_icmp_header)
{
    ip6_icmp_header->type = 0;
    ip6_icmp_header->code = 0;
    ip6_icmp_header->checksum = 0;
    ip6_icmp_header->data.m32[0] = 0;
}

ip6_icmp_header_type_t
ip6_icmp_header_get_type(ip6_icmp_header_t *ip6_icmp_header)
{
    return (ip6_icmp_header_type_t)ip6_icmp_header->type;
}

void
ip6_icmp_header_set_type(ip6_icmp_header_t *ip6_icmp_header, ip6_icmp_header_type_t type)
{
    ip6_icmp_header->type = (uint8_t)type;
}

ip6_icmp_header_code_t
ip6_icmp_header_get_code(ip6_icmp_header_t *ip6_icmp_header)
{
    return (ip6_icmp_header_code_t)ip6_icmp_header->code;
}

void
ip6_icmp_header_set_code(ip6_icmp_header_t *ip6_icmp_header, ip6_icmp_header_code_t code)
{
    ip6_icmp_header->code = (uint8_t)code;
}

uint16_t
ip6_icmp_header_get_checksum(ip6_icmp_header_t *ip6_icmp_header)
{
    return encoding_big_endian_swap16(ip6_icmp_header->checksum);
}

void
ip6_icmp_header_set_checksum(ip6_icmp_header_t *ip6_icmp_header, uint16_t checksum)
{
    ip6_icmp_header->checksum = encoding_big_endian_swap16(ip6_icmp_header->checksum);
}

uint16_t
ip6_icmp_header_get_id(ip6_icmp_header_t *ip6_icmp_header)
{
    return encoding_big_endian_swap16(ip6_icmp_header->data.m16[0]);
}

void
ip6_icmp_header_set_id(ip6_icmp_header_t *ip6_icmp_header, uint16_t id)
{
    ip6_icmp_header->data.m16[0] = encoding_big_endian_swap16(id);
}

uint16_t
ip6_icmp_header_get_sequence(ip6_icmp_header_t *ip6_icmp_header)
{
    return encoding_big_endian_swap16(ip6_icmp_header->data.m16[1]);
}

void
ip6_icmp_header_set_sequence(ip6_icmp_header_t *ip6_icmp_header, uint16_t sequence)
{
    ip6_icmp_header->data.m16[1] = encoding_big_endian_swap16(sequence);
}

uint8_t
ip6_icmp_header_get_checksum_offset(void)
{
    return offsetof(ns_icmp6_header_t, checksum);
}

uint8_t
ip6_icmp_header_get_data_offset(void)
{
    return offsetof(ns_icmp6_header_t, data);
}

// --- icmp handler functions
void
ip6_icmp_handler_ctor(ip6_icmp_handler_t *ip6_icmp_handler,
                      ns_icmp6_receive_callback_func_t callback,
                      void *context)
{
    ip6_icmp_handler->receive_callback = callback;
    ip6_icmp_handler->context = context;
    ip6_icmp_handler->next = NULL;
}

// --- icmp functions
void
ip6_icmp_ctor(ip6_icmp_t *ip6_icmp, void *instance)
{
    ip6_icmp->instance = instance;
    ip6_icmp->handlers = NULL;
    ip6_icmp->echo_sequence = 1;
    ip6_icmp->echo_mode = NS_ICMP6_ECHO_HANDLER_ALL;
}

message_t
ip6_icmp_new_message(ip6_icmp_t *ip6_icmp, uint16_t reserved)
{
#if 0 // TODO:
    return ip6_new_message(instance_get_ip6(ip6_icmp->instance), sizeof(ip6_icmp_header_t) + reserved, NULL);
#endif
    return NULL;
}

ns_error_t
ip6_icmp_register_handler(ip6_icmp_t *ip6_icmp, ip6_icmp_handler_t *handler)
{
    ns_error_t error = NS_ERROR_NONE;

    for (ip6_icmp_handler_t *cur = ip6_icmp->handlers; cur; cur = ip6_icmp_handler_get_next(cur)) {
        if (cur == handler) {
            EXIT_NOW(error = NS_ERROR_ALREADY);
        }
    }

    handler->next = ip6_icmp->handlers;
    ip6_icmp->handlers = handler;

exit:
    return error;
}

ns_error_t
ip6_icmp_send_echo_request(ip6_icmp_t *ip6_icmp,
                           message_t message,
                           const ip6_message_info_t *ip6_message_info,
                           uint16_t identifier)
{
    // TODO:
    return NS_ERROR_NONE;
}

ns_error_t
ip6_icmp_send_error(ip6_icmp_t *ip6_icmp,
                    ip6_icmp_header_type_t type,
                    ip6_icmp_header_code_t code,
                    const ip6_message_info_t *ip6_message_info,
                    const ip6_icmp_header_t *header)
{
    // TODO:
    return NS_ERROR_NONE;
}

ns_error_t
ip6_icmp_handle_message(ip6_icmp_t *ip6_icmp,
                        message_t message,
                        ip6_message_info_t *ip6_message_info)
{
    // TODO:
    return NS_ERROR_NONE;
}

ns_error_t
ip6_icmp_update_checksum(ip6_icmp_t *ip6_icmp,
                         message_t message,
                         uint16_t checksum)
{
    checksum = message_update_checksum(message,
                                       checksum,
                                       message_get_offset(message),
                                       message_get_length(message) - message_get_offset(message));

    if (checksum != 0xffff) {
        checksum = ~checksum;
    }

    checksum = encoding_big_endian_swap16(checksum);

    message_write(message,
                  message_get_offset(message) + ip6_icmp_header_get_checksum_offset(),
                  &checksum,
                  sizeof(checksum));

    return NS_ERROR_NONE;
}

ns_icmp6_echo_mode_t
ip6_icmp_get_echo_mode(ip6_icmp_t *ip6_icmp)
{
    return ip6_icmp->echo_mode;
}

void
ip6_icmp_set_echo_mode(ip6_icmp_t *ip6_icmp, ns_icmp6_echo_mode_t mode)
{
    ip6_icmp->echo_mode = mode;
}

bool
ip6_icmp_should_handle_echo_request(ip6_icmp_t *ip6_icmp,
                                    const ip6_message_info_t *ip6_message_info)
{
    // TODO:
    return false;
}

// --- private functions
#if 0
static void
ip6_icmp_handler_handle_receive_message(ip6_icmp_handler_t *ip6_icmp_handler,
                                        message_t message,
                                        const ip6_message_info_t *ip6_message_info,
                                        const ip6_icmp_header_t *icmp6_header)
{
    ip6_icmp_handler->receive_callback(ip6_icmp_handler->context,
                                       message,
                                       ip6_message_info,
                                       icmp6_header);
}
#endif

static ip6_icmp_handler_t *
ip6_icmp_handler_get_next(ip6_icmp_handler_t *ip6_icmp_handler)
{
    return (ip6_icmp_handler_t *)ip6_icmp_handler->next;
}

#if 0
static ns_error_t
ip6_icmp_handle_echo_request(ip6_icmp_t *ip6_icmp, message_t message)
{
    // TODO:
    return NS_ERROR_NONE;
}
#endif
