#ifndef NS_CORE_NET_UDP6_H_
#define NS_CORE_NET_UDP6_H_

#include "core/core-config.h"
#include "ns/udp.h"
#include "core/net/ip6_headers.h"

typedef ns_udp_receiver_t ip6_udp_receiver_t;
typedef ns_udp_socket_t ip6_udp_socket_t;

enum {
    IP6_UDP_DYNAMIC_PORT_MIN = 49152,
    IP6_UDP_DYNAMIC_PORT_MAX = 65535,
};

typedef struct _ip6_udp {
    void *instance;
    uint16_t ephemeral_port;
    ip6_udp_receiver_t *receivers;
    ip6_udp_socket_t *sockets;
#if NS_ENABLE_UDP_FORWARD
    void *udp_forwarder_context;
    ns_udp_forwarder_func_t udp_forwarder;
#endif
} ip6_udp_t;

typedef struct _ip6_udp_header {
    uint16_t source;
    uint16_t destination;
    uint16_t length;
    uint16_t checksum;
} ip6_udp_header_t;

// --- udp receiver functions
void
ip6_udp_receiver_ctor(ip6_udp_receiver_t *ip6_udp_receiver,
                      ns_udp_handler_func_t handler,
                      void *context);

// --- udp socket functions
void
ip6_udp_socket_ctor(ip6_udp_socket_t *ip6_udp_socket, ip6_udp_t *ip6_udp);

message_t
ip6_udp_socket_new_message(ip6_udp_socket_t *ip6_udp_socket,
                           uint16_t reserved,
                           const ns_message_settings_t *settings);

ns_error_t
ip6_udp_socket_open(ip6_udp_socket_t *ip6_udp_socket, ns_udp_receive_func_t handler, void *context);

ns_error_t
ip6_udp_socket_bind(ip6_udp_socket_t *ip6_udp_socket, const ip6_sock_addr_t *sock_addr);

ns_error_t
ip6_udp_socket_connect(ip6_udp_socket_t *ip6_udp_socket, const ip6_sock_addr_t *sock_addr);

ns_error_t
ip6_udp_socket_close(ip6_udp_socket_t *ip6_udp_socket);

ns_error_t
ip6_udp_socket_send_to(ip6_udp_socket_t *ip6_udp_socket,
                       message_t message,
                       const ip6_message_info_t *message_ifno);

ip6_sock_addr_t *
ip6_udp_socket_get_sock_name(ip6_udp_socket_t *ip6_udp_socket);

ip6_sock_addr_t *
ip6_udp_socket_get_peer_name(ip6_udp_socket_t *ip6_udp_socket);

// --- udp functions
void
ip6_udp_ctor(ip6_udp_t *ip6_udp, void *instance);

ns_error_t
ip6_udp_add_receiver(ip6_udp_t *ip6_udp, ip6_udp_receiver_t *receiver);

ns_error_t
ip6_udp_remove_receiver(ip6_udp_t *ip6_udp, ip6_udp_receiver_t *receiver);

ns_error_t
ip6_udp_add_socket(ip6_udp_t *ip6_udp, ip6_udp_socket_t *asocket);

ns_error_t
ip6_udp_remove_socket(ip6_udp_t *ip6_udp, ip6_udp_socket_t *asocket);

uint16_t
ip6_udp_get_ephemeral_port(ip6_udp_t *ip6_udp);

message_t
ip6_udp_new_message(ip6_udp_t *ip6_udp, uint16_t reserved, const ns_message_settings_t *settings);

ns_error_t
ip6_udp_send_datagram(ip6_udp_t *ip6_udp,
                      message_t message,
                      ip6_message_info_t *message_info,
                      ip6_ip_proto_t ip_proto);

ns_error_t
ip6_udp_handle_message(ip6_udp_t *ip6_udp, message_t message, ip6_message_info_t *message_info);

void
ip6_udp_handle_payload(ip6_udp_t *ip6_udp, message_t message, ip6_message_info_t *message_info);

ns_error_t
ip6_udp_update_checksum(ip6_udp_t *ip6_udp, message_t message, uint16_t *checksum);

#if NS_ENABLE_UDP_PLATFORM
ns_udp_socket_t *
ip6_udp_get_udp_sockets(ip6_udp_t *ip6_udp);
#endif

#if NS_ENABLE_UDP_FORWARD
void
ip6_udp_set_udp_forwarder(ip6_udp_t *ip6_udp, ns_udp_forwarder_func_t forwarder, void *context);
#endif

// --- udp header functions
uint16_t
ip6_udp_header_get_source_port(ip6_udp_header_t *ip6_udp_header);

void
ip6_udp_header_set_source_port(ip6_udp_header_t *ip6_udp_header, uint16_t port);

uint16_t
ip6_udp_header_get_destination_port(ip6_udp_header_t *ip6_udp_header);

void
ip6_udp_header_set_destination_port(ip6_udp_header_t *ip6_udp_header, uint16_t port);

uint16_t
ip6_udp_header_get_length(ip6_udp_header_t *ip6_udp_header);

void
ip6_udp_header_set_length(ip6_udp_header_t *ip6_udp_header, uint16_t length);

uint16_t
ip6_udp_header_get_checksum(ip6_udp_header_t *ip6_udp_header);

void
ip6_udp_header_set_checksum(ip6_udp_header_t *ip6_udp_header, uint16_t checksum);

uint8_t
ip6_udp_header_get_length_offset(void);

uint8_t
ip6_udp_header_get_checksum_offset(void);

#endif // NS_CORE_NET_UDP6_H_
