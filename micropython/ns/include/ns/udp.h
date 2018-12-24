#ifndef NS_UDP_H_
#define NS_UDP_H_

#include "ns/ip6.h"
#include "ns/message.h"

typedef bool (*ns_udp_handler_func_t)(void *context,
                                      const ns_message_t message,
                                      const ns_message_info_t *message_info);

typedef struct _ns_udp_receiver ns_udp_receiver_t;
struct _ns_udp_receiver {
    ns_udp_receiver_t *next;
    ns_udp_handler_func_t handler;
    void *context;
};

typedef void (*ns_udp_receive_func_t)(void *context,
                                      ns_message_t message,
                                      const ns_message_info_t *message_info);

typedef struct _ns_udp_socket ns_udp_socket_t;
struct _ns_udp_socket {
    ns_sock_addr_t sock_name;
    ns_sock_addr_t peer_name;
    ns_udp_receive_func_t handler;
    void *context;
    void *handle;
    ns_udp_socket_t *next;
};

ns_message_t
ns_udp_new_message(ns_instance_t instance, const ns_message_settings_t *settings);

ns_error_t
ns_udp_open(ns_instance_t instance, ns_udp_socket_t *socket, ns_udp_receive_func_t callback, void *context);

ns_error_t
ns_udp_close(ns_udp_socket_t *socket);

ns_error_t
ns_udp_bind(ns_udp_socket_t *socket, ns_sock_addr_t *sock_name);

ns_error_t
ns_udp_connect(ns_udp_socket_t *socket, ns_sock_addr_t *sock_name);

ns_error_t
ns_udp_send(ns_udp_socket_t *socket, ns_message_t message, const ns_message_info_t *message_info);

typedef void (*ns_udp_forwarder_func_t)(ns_message_t message,
                                        uint16_t peer_port,
                                        ns_ip6_addr_t *peer_addr,
                                        uint16_t sock_port,
                                        void *context);

void
ns_udp_forward_set_forwarder(ns_instance_t instance, ns_udp_forwarder_func_t forwarder, void *context);

void
ns_udp_forward_receive(ns_instance_t instance,
                       ns_message_t message,
                       uint16_t peer_port,
                       const ns_ip6_addr_t *peer_addr,
                       uint16_t sock_port);

ns_udp_socket_t *
ns_udp_get_sockets(ns_instance_t instance);

#endif // NS_UDP_H_
