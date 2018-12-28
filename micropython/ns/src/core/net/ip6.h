#ifndef NS_CORE_NET_IP6_H_
#define NS_CORE_NET_IP6_H_

#include "core/core-config.h"
#include "ns/ip6.h"
#include "ns/udp.h"
#include "core/common/encoding.h"
#include "core/common/message.h"
#include "core/net/icmp6.h"
#include "core/net/ip6_address.h"
#include "core/net/ip6_headers.h"
#include "core/net/ip6_mpl.h"
#include "core/net/ip6_routes.h"
#include "core/net/netif.h"
#include "core/net/socket.h"
#include "core/net/udp6.h"
#include <stddef.h>

enum {
    IP6_DEFAULT_HOP_LIMIT   = NS_CONFIG_IPV6_DEFAULT_HOP_LIMIT,
    IP6_MAX_DATAGRAM_LENGTH = NS_CONFIG_IPV6_DEFAULT_MAX_DATAGRAM,
};

enum {
    IP6_DEFAULT_MESSAGE_PRIORITY = MESSAGE_PRIO_NORMAL,
};

typedef struct _ip6 {
    void *instance;
    bool forwarding_enabled;
    bool is_receive_ip6_filter_enabled;
    ns_ip6_receive_callback_func_t receive_ip6_datagram_callback;
    void *receive_ip6_datagram_callback_context;
    ip6_netif_t *netif_list_head;
    priority_queue_t send_queue;
    tasklet_t send_queue_task;
    ip6_routes_t routes;
    ip6_icmp_t icmp;
    ip6_udp_t udp;
    ip6_mpl_t mpl;
} ip6_t;

// --- ip6 functions
void
ip6_ctor(ip6_t *ip6, void *instance);

message_t
ip6_new_message(ip6_t *ip6, uint16_t reserved, const ns_message_settings_t *settings);

uint8_t
ip6_priority_to_dscp(uint8_t priority);

uint8_t
ip6_dscp_to_priority(uint8_t dscp);

ns_error_t
ip6_send_datagram(ip6_t *ip6, message_t message, ip6_message_info_t *message_info, ip6_ip_proto_t ip_proto);

ns_error_t
ip6_send_raw(ip6_t *ip6, message_t message, int8_t interface_id);

ns_error_t
ip6_handle_datagram(ip6_t *ip6,
                    message_t message,
                    ip6_netif_t *netif,
                    int8_t interface_id,
                    const void *link_message_info,
                    bool from_ncp_host);

void
ip6_enqueue_datagram(ip6_t *ip6, message_t message);

uint16_t
ip6_update_checksum(uint16_t checksum, const ip6_addr_t *addr);

uint16_t
ip6_compute_pseudo_header_checksum(const ip6_addr_t *source,
                                   const ip6_addr_t *destination,
                                   uint16_t length,
                                   ip6_ip_proto_t proto);

void
ip6_set_receive_datagram_callback(ip6_t *ip6, ns_ip6_receive_callback_func_t callback, void *callback_context);

bool
ip6_is_receive_ip6_filter_enabled(ip6_t *ip6);

void
ip6_set_receive_ip6_filter_enabled(ip6_t *ip6, bool enabled);

bool
ip6_is_forwarding_enabled(ip6_t *ip6);

void
ip6_set_forwarding_enabled(ip6_t *ip6, bool enabled);

ns_error_t
ip6_add_netif(ip6_t *ip6, ip6_netif_t *anetif);

ns_error_t
ip6_remove_netif(ip6_t *ip6, ip6_netif_t *anetif);

ip6_netif_t *
ip6_get_netif_list(ip6_t *ip6);

ip6_netif_t *
ip6_get_netif_by_id(ip6_t *ip6, int8_t interface_id);

bool
ip6_is_unicast_addr(ip6_t *ip6, const ip6_addr_t *addr);

ip6_netif_unicast_addr_t *
ip6_select_source_addr(ip6_t *ip6, ip6_message_info_t *message_info);

int8_t
ip6_get_on_link_netif(ip6_t *ip6, const ip6_addr_t *addr);

priority_queue_t *
ip6_get_send_queue(ip6_t *ip6);

ip6_routes_t *
ip6_get_routes(ip6_t *ip6);

ip6_icmp_t *
ip6_get_icmp(ip6_t *ip6);

ip6_udp_t *
ip6_get_udp(ip6_t *ip6);

ip6_mpl_t *
ip6_get_mpl(ip6_t *ip6);

const char *
ip6_ip_proto_to_string(ip6_ip_proto_t ip_proto);

#endif // NS_CORE_NET_IP6_H_
