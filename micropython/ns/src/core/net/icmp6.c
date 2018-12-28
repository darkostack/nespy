#include "core/net/icmp6.h"
#include "core/common/code_utils.h"
#include "core/common/debug.h"
#include "core/common/instance.h"
#include "core/common/message.h"
#include "core/net/ip6.h"
#include <stddef.h>

// --- private functions declarations
static void
ip6_icmp_handler_handle_receive_message(ip6_icmp_handler_t *ip6_icmp_handler,
                                        message_t message,
                                        const ip6_message_info_t *message_info,
                                        const ip6_icmp_header_t *icmp6_header);

static ip6_icmp_handler_t *
ip6_icmp_handler_get_next(ip6_icmp_handler_t *ip6_icmp_handler);

static ns_error_t
ip6_icmp_handle_echo_request(ip6_icmp_t *ip6_icmp,
                             message_t request_message,
                             const ip6_message_info_t *message_info);

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
    return ip6_new_message(instance_get_ip6(ip6_icmp->instance), sizeof(ip6_icmp_header_t) + reserved, NULL);
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
                           const ip6_message_info_t *message_info,
                           uint16_t identifier)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_message_info_t message_info_local;
    ip6_icmp_header_t icmp_header;

    message_info_local = *(ip6_message_info_t *)message_info;

    ip6_icmp_header_init(&icmp_header);
    ip6_icmp_header_set_type(&icmp_header, IP6_ICMP_HEADER_TYPE_ECHO_REQUEST);
    ip6_icmp_header_set_id(&icmp_header, identifier);
    ip6_icmp_header_set_sequence(&icmp_header, ip6_icmp->echo_sequence++);

    SUCCESS_OR_EXIT(error = message_prepend(message, &icmp_header, sizeof(icmp_header)));
    message_set_offset(message, 0);
    SUCCESS_OR_EXIT(error = ip6_send_datagram(instance_get_ip6(ip6_icmp->instance),
                                              message,
                                              &message_info_local,
                                              IP6_IP_PROTO_ICMP6));

    ns_log_info_icmp("send echo request: (seq = %d)", ip6_icmp_header_get_sequence(&icmp_header));

exit:
    return error;
}

ns_error_t
ip6_icmp_send_error(ip6_icmp_t *ip6_icmp,
                    ip6_icmp_header_type_t type,
                    ip6_icmp_header_code_t code,
                    const ip6_message_info_t *message_info,
                    const ip6_header_t *header)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_message_info_t message_info_local;
    message_t message = NULL;
    ip6_icmp_header_t icmp_header;

    message_info_local = *(ip6_message_info_t *)message_info;

    VERIFY_OR_EXIT((message = ip6_new_message(instance_get_ip6(ip6_icmp->instance), 0, NULL)) != NULL,
                   error = NS_ERROR_NO_BUFS);
    SUCCESS_OR_EXIT(error = message_set_length(message, sizeof(icmp_header) + sizeof(*header)));

    message_write(message, sizeof(icmp_header), header, sizeof(*header));

    ip6_icmp_header_init(&icmp_header);
    ip6_icmp_header_set_type(&icmp_header, type);
    ip6_icmp_header_set_code(&icmp_header, code);
    message_write(message, 0, &icmp_header, sizeof(icmp_header));

    SUCCESS_OR_EXIT(error = ip6_send_datagram(instance_get_ip6(ip6_icmp->instance),
                                              message,
                                              &message_info_local,
                                              IP6_IP_PROTO_ICMP6));

    ns_log_info_icmp("sent icmpv6 error");

exit:
    if (error != NS_ERROR_NONE && message != NULL) {
        message_free(message);
    }

    return error;
}

ns_error_t
ip6_icmp_handle_message(ip6_icmp_t *ip6_icmp,
                        message_t message,
                        ip6_message_info_t *message_info)
{
    ns_error_t error = NS_ERROR_NONE;
    uint16_t payload_length;
    ip6_icmp_header_t icmp_header;
    uint16_t checksum;

    VERIFY_OR_EXIT(message_read(message, message_get_offset(message), &icmp_header, sizeof(icmp_header)) ==
                   sizeof(icmp_header), error = NS_ERROR_PARSE);
    payload_length = message_get_length(message) - message_get_offset(message);

    // verify checksum
    checksum = ip6_compute_pseudo_header_checksum(ip6_message_info_get_peer_addr(message_info),
                                                  ip6_message_info_get_sock_addr(message_info),
                                                  payload_length,
                                                  IP6_IP_PROTO_ICMP6);
    checksum = message_update_checksum(message, checksum, message_get_offset(message), payload_length);
    VERIFY_OR_EXIT(checksum == 0xffff, error = NS_ERROR_PARSE);

    if (ip6_icmp_header_get_type(&icmp_header) == IP6_ICMP_HEADER_TYPE_ECHO_REQUEST) {
        SUCCESS_OR_EXIT(error = ip6_icmp_handle_echo_request(ip6_icmp, message, message_info));
    }

    message_move_offset(message, sizeof(icmp_header));

    for (ip6_icmp_handler_t *handler = ip6_icmp->handlers; handler;
         handler = ip6_icmp_handler_get_next(handler)) {
        ip6_icmp_handler_handle_receive_message(handler, message, message_info, &icmp_header);
    }

exit:
    return error;
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
                                    const ip6_message_info_t *message_info)
{
    bool rval = false;

    switch (ip6_icmp->echo_mode) {
    case NS_ICMP6_ECHO_HANDLER_DISABLED:
        rval = false;
        break;
    case NS_ICMP6_ECHO_HANDLER_UNICAST_ONLY:
        rval = !ip6_addr_is_multicast((ip6_addr_t *)&message_info->sock_addr);
        break;
    case NS_ICMP6_ECHO_HANDLER_MULTICAST_ONLY:
        rval = ip6_addr_is_multicast((ip6_addr_t *)&message_info->sock_addr);
        break;
    case NS_ICMP6_ECHO_HANDLER_ALL:
        rval = true;
        break;
    }

    return rval;
}

// --- private functions
static void
ip6_icmp_handler_handle_receive_message(ip6_icmp_handler_t *ip6_icmp_handler,
                                        message_t message,
                                        const ip6_message_info_t *message_info,
                                        const ip6_icmp_header_t *icmp6_header)
{
    ip6_icmp_handler->receive_callback(ip6_icmp_handler->context, message, message_info, icmp6_header);
}

static ip6_icmp_handler_t *
ip6_icmp_handler_get_next(ip6_icmp_handler_t *ip6_icmp_handler)
{
    return (ip6_icmp_handler_t *)ip6_icmp_handler->next;
}

static ns_error_t
ip6_icmp_handle_echo_request(ip6_icmp_t *ip6_icmp,
                             message_t request_message,
                             const ip6_message_info_t *message_info)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_icmp_header_t icmp_header;
    message_t reply_message = NULL;
    ip6_message_info_t reply_message_info;
    uint16_t payload_length;

    // always handle echo request destined for RLOC and ALOC
    VERIFY_OR_EXIT(ip6_icmp_should_handle_echo_request(ip6_icmp, message_info) ||
                   ip6_addr_is_routing_locator((ip6_addr_t *)&message_info->sock_addr) ||
                   ip6_addr_is_anycast_routing_locator((ip6_addr_t *)&message_info->sock_addr));

    ns_log_info_icmp("received echo request");

    ip6_icmp_header_init(&icmp_header);
    ip6_icmp_header_set_type(&icmp_header, IP6_ICMP_HEADER_TYPE_ECHO_REPLY);

    if ((reply_message = ip6_new_message(instance_get_ip6(ip6_icmp->instance), 0, NULL)) == NULL) {
        ns_log_debg_icmp("failed to allocate a new message");
        EXIT_NOW();
    }

    payload_length = message_get_length(request_message) - message_get_offset(request_message) - ip6_icmp_header_get_data_offset();
    SUCCESS_OR_EXIT(error = message_set_length(reply_message, ip6_icmp_header_get_data_offset() + payload_length));

    message_write(reply_message, 0, &icmp_header, ip6_icmp_header_get_data_offset());
    message_copy_to(request_message,
                    message_get_offset(request_message) + ip6_icmp_header_get_data_offset(),
                    ip6_icmp_header_get_data_offset(),
                    payload_length,
                    reply_message);

    ip6_message_info_set_peer_addr(&reply_message_info, message_info->peer_addr);

    if (!ip6_addr_is_multicast((ip6_addr_t *)&message_info->sock_addr)) {
        ip6_message_info_set_sock_addr(&reply_message_info, message_info->sock_addr);
    }

    ip6_message_info_set_interface_id(&reply_message_info, message_info->interface_id);

    SUCCESS_OR_EXIT(error = ip6_send_datagram(instance_get_ip6(ip6_icmp->instance),
                                              reply_message,
                                              &reply_message_info,
                                              IP6_IP_PROTO_ICMP6));

    message_read(reply_message, message_get_offset(reply_message), &icmp_header, sizeof(icmp_header));

    ns_log_info_icmp("sent echo reply (seq = %d)", ip6_icmp_header_get_sequence(&icmp_header));

exit:
    if (error != NS_ERROR_NONE && reply_message != NULL) {
        message_free(reply_message);
    }

    return error;
}
