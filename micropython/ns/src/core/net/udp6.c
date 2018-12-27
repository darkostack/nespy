#include "core/net/udp6.h"
// TODO; include "ns/platform/udp.h"
#include "core/common/code_utils.h"
#include "core/common/encoding.h"
#include "core/common/instance.h"
#include "core/net/ip6.h"
#include <stddef.h>

// --- private functions declarations
static ip6_udp_receiver_t *
ip6_udp_receiver_get_next(ip6_udp_receiver_t *ip6_udp_receiver);

static void
ip6_udp_receiver_set_next(ip6_udp_receiver_t *ip6_udp_receiver,
                          ip6_udp_receiver_t *receiver);

static bool
ip6_udp_receiver_handle_message(ip6_udp_receiver_t *ip6_udp_receiver,
                                message_t message,
                                const ip6_message_info_t *message_info);

static ip6_udp_socket_t *
ip6_udp_socket_get_next(ip6_udp_socket_t *ip6_udp_socket);

static void
ip6_udp_socket_set_next(ip6_udp_socket_t *ip6_udp_socket, ip6_udp_socket_t *socket);

static void
ip6_udp_socket_handle_udp_receive(ip6_udp_socket_t *ip6_udp_socket,
                                  message_t message,
                                  const ip6_message_info_t *message_info);

static ip6_udp_t *
ip6_udp_socket_get_udp(ip6_udp_socket_t *ip6_udp_socket);

// --- udp receiver functions
void
ip6_udp_receiver_ctor(ip6_udp_receiver_t *ip6_udp_receiver,
                      ns_udp_handler_func_t handler,
                      void *context)
{
    ip6_udp_receiver->next = NULL;
    ip6_udp_receiver->handler = handler;
    ip6_udp_receiver->context = context;
}

// --- udp socket functions
void
ip6_udp_socket_ctor(ip6_udp_socket_t *ip6_udp_socket, ip6_udp_t *ip6_udp)
{
    ip6_udp_socket->instance = ip6_udp->instance;
    ip6_udp_socket->handle = NULL;
}

message_t
ip6_udp_socket_new_message(ip6_udp_socket_t *ip6_udp_socket,
                           uint16_t reserved,
                           const ns_message_settings_t *settings)
{
    return ip6_udp_new_message(ip6_udp_socket_get_udp(ip6_udp_socket), reserved, settings);
}

ns_error_t
ip6_udp_socket_open(ip6_udp_socket_t *ip6_udp_socket, ns_udp_receive_func_t handler, void *context)
{
    ns_error_t error;

    memset(&ip6_udp_socket->sock_name, 0, sizeof(ip6_udp_socket->sock_name));
    memset(&ip6_udp_socket->peer_name, 0, sizeof(ip6_udp_socket->peer_name));

    ip6_udp_socket->handler = handler;
    ip6_udp_socket->context = context;

#if NS_ENABLE_PLATFORM_UDP
    // TODO: SUCCESS_OR_EXIT(error = ns_plat_udp_socket(ip6_udp_socket);
#endif
    SUCCESS_OR_EXIT(error = ip6_udp_add_socket(ip6_udp_socket_get_udp(ip6_udp_socket), ip6_udp_socket));

exit:
    return error;
}

ns_error_t
ip6_udp_socket_bind(ip6_udp_socket_t *ip6_udp_socket, const ip6_sock_addr_t *sock_addr)
{
    // TODO:
    return NS_ERROR_NONE;
}

ns_error_t
ip6_udp_socket_connect(ip6_udp_socket_t *ip6_udp_socket, const ip6_sock_addr_t *sock_addr)
{
    // TODO:
    return NS_ERROR_NONE;
}

ns_error_t
ip6_udp_socket_close(ip6_udp_socket_t *ip6_udp_socket)
{
    ns_error_t error = NS_ERROR_NONE;

#if NS_ENABLE_PLATFORM_UDP
    // TODO: SUCCESS_OR_EXIT(error = ns_plat_udp_close(ip6_udp_socket));
#endif
    SUCCESS_OR_EXIT(error = ip6_udp_remove_socket(ip6_udp_socket_get_udp(ip6_udp_socket), ip6_udp_socket));
    memset(&ip6_udp_socket->sock_name, 0, sizeof(ip6_udp_socket->sock_name));
    memset(&ip6_udp_socket->peer_name, 0, sizeof(ip6_udp_socket->peer_name));

exit:
    return error;
}

ns_error_t
ip6_udp_socket_send_to(ip6_udp_socket_t *ip6_udp_socket,
                       message_t message,
                       const ip6_message_info_t *message_ifno)
{
    // TODO:
    return NS_ERROR_NONE;
}

ip6_sock_addr_t *
ip6_udp_socket_get_sock_name(ip6_udp_socket_t *ip6_udp_socket)
{
    return (ip6_sock_addr_t *)&ip6_udp_socket->sock_name;
}

ip6_sock_addr_t *
ip6_udp_socket_get_peer_name(ip6_udp_socket_t *ip6_udp_socket)
{
    return (ip6_sock_addr_t *)&ip6_udp_socket->peer_name;
}

// --- udp functions
void
ip6_udp_ctor(ip6_udp_t *ip6_udp, void *instance)
{
    ip6_udp->instance = instance;
    ip6_udp->ephemeral_port = IP6_UDP_DYNAMIC_PORT_MIN;
    ip6_udp->receivers = NULL;
    ip6_udp->sockets = NULL;
}

ns_error_t
ip6_udp_add_receiver(ip6_udp_t *ip6_udp, ip6_udp_receiver_t *areceiver)
{
    for (ip6_udp_receiver_t *cur = ip6_udp->receivers; cur; cur = ip6_udp_receiver_get_next(cur)) {
        if (cur == areceiver) {
            EXIT_NOW();
        }
    }

    ip6_udp_receiver_set_next(areceiver, ip6_udp->receivers);
    ip6_udp->receivers = areceiver;

exit:
    return NS_ERROR_NONE;
}

ns_error_t
ip6_udp_remove_receiver(ip6_udp_t *ip6_udp, ip6_udp_receiver_t *areceiver)
{
    if (ip6_udp->receivers == areceiver) {
        ip6_udp->receivers = ip6_udp_receiver_get_next(ip6_udp->receivers);
    } else {
        for (ip6_udp_receiver_t *handler = ip6_udp->receivers; handler;
             handler = ip6_udp_receiver_get_next(handler)) {
            if (ip6_udp_receiver_get_next(handler) == areceiver) {
                ip6_udp_receiver_set_next(handler, ip6_udp_receiver_get_next(areceiver));
                break;
            }
        }
    }

    ip6_udp_receiver_set_next(areceiver, NULL);

    return NS_ERROR_NONE;
}

ns_error_t
ip6_udp_add_socket(ip6_udp_t *ip6_udp, ip6_udp_socket_t *asocket)
{
    for (ip6_udp_socket_t *cur = ip6_udp->sockets; cur; cur = ip6_udp_socket_get_next(cur)) {
        if (cur == asocket) {
            EXIT_NOW();
        }
    }

    ip6_udp_socket_set_next(asocket, ip6_udp->sockets);
    ip6_udp->sockets = asocket;

exit:
    return NS_ERROR_NONE;
}

ns_error_t
ip6_udp_remove_socket(ip6_udp_t *ip6_udp, ip6_udp_socket_t *asocket)
{
    if (ip6_udp->sockets == asocket) {
        ip6_udp->sockets = ip6_udp_socket_get_next(ip6_udp->sockets);
    } else {
        for (ip6_udp_socket_t *socket = ip6_udp->sockets; socket; socket = ip6_udp_socket_get_next(socket)) {
            if (ip6_udp_socket_get_next(socket) == asocket) {
                ip6_udp_socket_set_next(socket, ip6_udp_socket_get_next(asocket));
                break;
            }
        }
    }

    ip6_udp_socket_set_next(asocket, NULL);

    return NS_ERROR_NONE;
}

uint16_t
ip6_udp_get_ephemeral_port(ip6_udp_t *ip6_udp)
{
    uint16_t rval = ip6_udp->ephemeral_port;

    if (ip6_udp->ephemeral_port < IP6_UDP_DYNAMIC_PORT_MAX) {
        ip6_udp->ephemeral_port++;
    } else {
        ip6_udp->ephemeral_port = IP6_UDP_DYNAMIC_PORT_MIN;
    }

    return rval;
}

message_t
ip6_udp_new_message(ip6_udp_t *ip6_udp, uint16_t reserved, const ns_message_settings_t *settings)
{
    // TODO: return ip6_new_message(instance_get_ip6(ip6_udp->instance), sizeof(ip6_udp_header_t) + reserved, settings);
    return NULL;
}

ns_error_t
ip6_udp_send_datagram(ip6_udp_t *ip6_udp,
                      message_t message,
                      ip6_message_info_t *message_info,
                      ip6_ip_proto_t ip_proto)
{
    ns_error_t error = NS_ERROR_NONE;

#if NS_ENABLE_UDP_FORWARD
    if (ip6_message_info_get_interface_id(message_info) == NS_NETIF_INTERFACE_ID_HOST) {
        VERIFY_OR_EXIT(ip6_udp->udp_forwarder != NULL, error = NS_ERROR_NO_ROUTE);
        ip6_udp->udp_forwarder(message,
                               ip6_message_info_get_peer_port(message_info),
                               ip6_message_info_get_peer_addr(message_info),
                               ip6_message_info_get_sock_port(message_info),
                               ip6_udp->udp_forwarder_context);
        // message is consumed by the callback
    } else
#endif
    {
        ip6_udp_header_t udp_header;

        ip6_udp_header_set_source_port(&udp_header, message_info->sock_port);
        ip6_udp_header_set_destination_port(&udp_header, message_info->peer_port);
        ip6_udp_header_set_length(&udp_header, sizeof(udp_header) + message_get_length(message));
        ip6_udp_header_set_checksum(&udp_header, 0);

        SUCCESS_OR_EXIT(error = message_prepend(message, &udp_header, sizeof(udp_header)));
        message_set_offset(message, 0);

        // TODO: error = ip6_send_datagram(instance_get_ip6(ip6_udp->instance), message, message_info, ip_proto);
    }

exit:
    return error;
}

ns_error_t
ip6_udp_handle_message(ip6_udp_t *ip6_udp, message_t message, ip6_message_info_t *message_info)
{
    ns_error_t error = NS_ERROR_NONE;

    // TODO:

    for (ip6_udp_receiver_t *receiver = ip6_udp->receivers; receiver;
         receiver = ip6_udp_receiver_get_next(receiver)) {
        VERIFY_OR_EXIT(!ip6_udp_receiver_handle_message(receiver, message, message_info));
    }

    ip6_udp_handle_payload(ip6_udp, message, message_info);

exit:
    return error;
}

void
ip6_udp_handle_payload(ip6_udp_t *ip6_udp, message_t message, ip6_message_info_t *message_info)
{
    // finc socket
    for (ip6_udp_socket_t *socket = ip6_udp->sockets; socket; socket = ip6_udp_socket_get_next(socket)) {
        if (((ip6_sock_addr_t *)ip6_udp_socket_get_sock_name(socket))->port !=
            ip6_message_info_get_sock_port(message_info)) {
            continue;
        }
        if (((ip6_sock_addr_t *)ip6_udp_socket_get_sock_name(socket))->scope_id != 0 &&
            ((ip6_sock_addr_t *)ip6_udp_socket_get_sock_name(socket))->scope_id !=
            ip6_message_info_get_interface_id(message_info)) {
            continue;
        }
        if (!ip6_addr_is_multicast(ip6_message_info_get_sock_addr(message_info)) &&
            !ip6_addr_is_unspecified(ip6_sock_addr_get_addr(ip6_udp_socket_get_sock_name(socket))) &&
            !ip6_addr_is_equal(ip6_sock_addr_get_addr(ip6_udp_socket_get_sock_name(socket)),
                               ip6_message_info_get_sock_addr(message_info))) {
            continue;
        }
        // verify source of connected socket
        if (((ip6_sock_addr_t *)ip6_udp_socket_get_peer_name(socket))->port != 0) {
            if (((ip6_sock_addr_t *)ip6_udp_socket_get_peer_name(socket))->port !=
                ip6_message_info_get_peer_port(message_info)) {
                continue;
            }
            if (!ip6_addr_is_unspecified(ip6_sock_addr_get_addr(ip6_udp_socket_get_peer_name(socket))) &&
                !ip6_addr_is_equal(ip6_sock_addr_get_addr(ip6_udp_socket_get_peer_name(socket)),
                                   ip6_message_info_get_peer_addr(message_info))) {
                continue;
            }
        }
        ip6_udp_socket_handle_udp_receive(socket, message, (const ip6_message_info_t *)message_info);
    }
}

ns_error_t
ip6_udp_update_checksum(ip6_udp_t *ip6_udp, message_t message, uint16_t *checksum)
{
    *checksum = message_update_checksum(message,
                                        *checksum,
                                        message_get_offset(message),
                                        message_get_length(message) - message_get_offset(message));

    if (*checksum != 0xffff) {
        *checksum = ~(*checksum);
    }

    *checksum = encoding_big_endian_swap16(*checksum);
    message_write(message,
                  message_get_offset(message) + ip6_udp_header_get_checksum_offset(),
                  checksum,
                  sizeof(*checksum));

    return NS_ERROR_NONE;
}

#if NS_ENABLE_UDP_PLATFORM
ns_udp_socket_t *
ip6_udp_get_udp_sockets(ip6_udp_t *ip6_udp)
{
    return ip6_udp->sockets;
}
#endif

#if NS_ENABLE_UDP_FORWARD
void
ip6_udp_set_udp_forwarder(ip6_udp_t *ip6_udp, ns_udp_forwarder_func_t forwarder, void *context)
{
    ip6_udp->udp_forwarder = forwarder;
    ip6_udp->udp_forwarder_context = context;
}
#endif

// --- udp header functions
uint16_t
ip6_udp_header_get_source_port(ip6_udp_header_t *ip6_udp_header)
{
    return encoding_big_endian_swap16(ip6_udp_header->source);
}

void
ip6_udp_header_set_source_port(ip6_udp_header_t *ip6_udp_header, uint16_t port)
{
    ip6_udp_header->source = encoding_big_endian_swap16(port);
}

uint16_t
ip6_udp_header_get_destination_port(ip6_udp_header_t *ip6_udp_header)
{
    return encoding_big_endian_swap16(ip6_udp_header->destination);
}

void
ip6_udp_header_set_destination_port(ip6_udp_header_t *ip6_udp_header, uint16_t port)
{
    ip6_udp_header->destination = encoding_big_endian_swap16(port);
}

uint16_t
ip6_udp_header_get_length(ip6_udp_header_t *ip6_udp_header)
{
    return encoding_big_endian_swap16(ip6_udp_header->length);
}

void
ip6_udp_header_set_length(ip6_udp_header_t *ip6_udp_header, uint16_t length)
{
    ip6_udp_header->length = encoding_big_endian_swap16(length);
}

uint16_t
ip6_udp_header_get_checksum(ip6_udp_header_t *ip6_udp_header)
{
    return encoding_big_endian_swap16(ip6_udp_header->checksum);
}

void
ip6_udp_header_set_checksum(ip6_udp_header_t *ip6_udp_header, uint16_t checksum)
{
    ip6_udp_header->checksum = encoding_big_endian_swap16(checksum);
}

uint8_t
ip6_udp_header_get_length_offset(void)
{
    return offsetof(ip6_udp_header_t, length);
}

uint8_t
ip6_udp_header_get_checksum_offset(void)
{
    return offsetof(ip6_udp_header_t, checksum);
}

// --- private functions
static ip6_udp_receiver_t *
ip6_udp_receiver_get_next(ip6_udp_receiver_t *ip6_udp_receiver)
{
    return (ip6_udp_receiver_t *)ip6_udp_receiver->next;
}

static void
ip6_udp_receiver_set_next(ip6_udp_receiver_t *ip6_udp_receiver,
                          ip6_udp_receiver_t *receiver)
{
    ip6_udp_receiver->next = (ns_udp_receiver_t *)receiver;
}

static bool
ip6_udp_receiver_handle_message(ip6_udp_receiver_t *ip6_udp_receiver,
                                message_t message,
                                const ip6_message_info_t *message_info)
{
    return ip6_udp_receiver->handler(ip6_udp_receiver->context,
                                     (const ns_message_t)message,
                                     message_info);
}

static ip6_udp_socket_t *
ip6_udp_socket_get_next(ip6_udp_socket_t *ip6_udp_socket)
{
    return (ip6_udp_socket_t *)ip6_udp_socket->next;
}

static void
ip6_udp_socket_set_next(ip6_udp_socket_t *ip6_udp_socket, ip6_udp_socket_t *socket)
{
    ip6_udp_socket->next = (ns_udp_socket_t *)socket;
}

static void
ip6_udp_socket_handle_udp_receive(ip6_udp_socket_t *ip6_udp_socket,
                                  message_t message,
                                  const ip6_message_info_t *message_info)
{
    ip6_udp_socket->handler(ip6_udp_socket->context, message, message_info);
}

static ip6_udp_t *
ip6_udp_socket_get_udp(ip6_udp_socket_t *ip6_udp_socket)
{
    // TODO: return ip6_get_udp(instance_get_ip6(ip6_udp_socket->instance))
    return NULL;
}
