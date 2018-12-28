#include "core/net/ip6.h"
#include "core/common/code_utils.h"
#include "core/common/debug.h"
#include "core/common/instance.h"
#include "core/common/logging.h"
#include "core/thread/mle.h"

// --- private functions declarations
static void
handle_send_queue(void *tasklet);

static void
ip6_handle_send_queue(ip6_t *ip6);

static ns_error_t
ip6_process_receive_callback(ip6_t *ip6,
                             message_t message,
                             ip6_message_info_t *message_info,
                             uint8_t ip_proto,
                             bool from_ncp_host);

static ns_error_t
ip6_handle_extension_headers(ip6_t *ip6,
                             message_t message,
                             ip6_header_t *header,
                             uint8_t *next_header,
                             bool forward,
                             bool receive);

static ns_error_t
ip6_handle_fragment(ip6_t *ip6, message_t message);

static ns_error_t
ip6_add_mpl_option(ip6_t *ip6, message_t message, ip6_header_t *header);

static ns_error_t
ip6_add_tunneled_mpl_option(ip6_t *ip6,
                            message_t message,
                            ip6_header_t *header,
                            ip6_message_info_t *message_info);

static ns_error_t
ip6_insert_mpl_option(ip6_t *ip6,
                      message_t message,
                      ip6_header_t *header,
                      ip6_message_info_t *message_info);

static ns_error_t
ip6_remove_mpl_option(ip6_t *ip6, message_t message);

static ns_error_t
ip6_handle_options(ip6_t *ip6, message_t message, ip6_header_t *header, bool forward);

static ns_error_t
ip6_handle_payload(ip6_t *ip6, message_t message, ip6_message_info_t *message_info, uint8_t ip_proto);

static int8_t
ip6_find_forward_interface_id(ip6_t *ip6, ip6_message_info_t *message_info);

// --- ip6 functions
void
ip6_ctor(ip6_t *ip6, void *instance)
{
    ip6->instance = instance;
    ip6->forwarding_enabled = false;
    ip6->is_receive_ip6_filter_enabled = false;
    ip6->receive_ip6_datagram_callback = NULL;
    ip6->receive_ip6_datagram_callback_context = NULL;
    ip6->netif_list_head = NULL;
    message_priority_queue_ctor(&ip6->send_queue);
    tasklet_ctor(instance, &ip6->send_queue_task, &handle_send_queue, ip6);
    ip6_routes_ctor(&ip6->routes, instance);
    ip6_icmp_ctor(&ip6->icmp, instance);
    ip6_udp_ctor(&ip6->udp, instance);
    ip6_mpl_ctor(&ip6->mpl, instance);
}

message_t
ip6_new_message(ip6_t *ip6, uint16_t reserved, const ns_message_settings_t *settings)
{
    return message_new_set(MESSAGE_TYPE_IP6,
            sizeof(ip6_header_t) + sizeof(ip6_hop_by_hop_header_t) + sizeof(ip6_option_mpl_t) + reserved,
            settings);
}

uint8_t
ip6_priority_to_dscp(uint8_t priority)
{
    ip6_ip_dscp_cs_t dscp = IP6_DSCP_CS0;

    switch (priority) {
    case MESSAGE_PRIO_LOW:
        dscp = IP6_DSCP_CS1;
        break;
    case MESSAGE_PRIO_NORMAL:
        dscp = IP6_DSCP_CS0;
        break;
    case MESSAGE_PRIO_HIGH:
        dscp = IP6_DSCP_CS4;
        break;
    }

    return (uint8_t)dscp;
}

uint8_t
ip6_dscp_to_priority(uint8_t dscp)
{
    uint8_t priority;
    ip6_ip_dscp_cs_t cs = dscp & IP6_DSCP_CSMASK;

    switch (cs) {
    case IP6_DSCP_CS1:
    case IP6_DSCP_CS2:
        priority = MESSAGE_PRIO_LOW;
        break;
    case IP6_DSCP_CS0:
    case IP6_DSCP_CS3:
        priority = MESSAGE_PRIO_NORMAL;
        break;
    case IP6_DSCP_CS4:
    case IP6_DSCP_CS5:
    case IP6_DSCP_CS6:
    case IP6_DSCP_CS7:
        priority = MESSAGE_PRIO_HIGH;
        break;
    default:
        priority = MESSAGE_PRIO_NORMAL;
        break;
    };

    return priority;
}

ns_error_t
ip6_send_datagram(ip6_t *ip6, message_t message, ip6_message_info_t *message_info, ip6_ip_proto_t ip_proto)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_header_t header;
    uint16_t payload_length = message_get_length(message);
    uint16_t checksum;
    ip6_netif_unicast_addr_t *source;

    ip6_header_init(&header);
    ip6_header_set_dscp(&header, ip6_priority_to_dscp(message_get_priority(message)));
    ip6_header_set_payload_length(&header, payload_length);
    ip6_header_set_hop_limit(&header, message_info->hop_limit ?
                                      message_info->hop_limit : (uint8_t)IP6_DEFAULT_HOP_LIMIT);

    if (ip6_addr_is_unspecified(ip6_message_info_get_sock_addr(message_info)) ||
        ip6_addr_is_multicast(ip6_message_info_get_sock_addr(message_info))) {
        VERIFY_OR_EXIT((source = ip6_select_source_addr(ip6, message_info)) != NULL,
                       error = NS_ERROR_INVALID_SOURCE_ADDRESS);
        ip6_header_set_source(&header, ip6_netif_unicast_addr_get_addr(source));
    } else {
        ip6_header_set_source(&header, ip6_message_info_get_sock_addr(message_info));
    }

    ip6_header_set_destination(&header, ip6_message_info_get_peer_addr(message_info));

    if (ip6_addr_is_link_local(ip6_header_get_destination(&header)) ||
        ip6_addr_is_link_local_multicast(ip6_header_get_destination(&header))) {
        VERIFY_OR_EXIT(ip6_message_info_get_interface_id(message_info) != 0, error = NS_ERROR_DROP);
    }

    if (ip6_addr_is_realm_local_multicast(ip6_message_info_get_peer_addr(message_info))) {
        SUCCESS_OR_EXIT(error = ip6_add_mpl_option(ip6, message, &header));
    }

    SUCCESS_OR_EXIT(error = message_prepend(message, &header, sizeof(header)));

    // compute checksum
    checksum = ip6_compute_pseudo_header_checksum(ip6_header_get_source(&header),
                                                  ip6_header_get_destination(&header),
                                                  payload_length,
                                                  ip_proto);

    switch (ip_proto) {
    case IP6_IP_PROTO_UDP:
        SUCCESS_OR_EXIT(error = ip6_udp_update_checksum(&ip6->udp, message, checksum));
        break;
    case IP6_IP_PROTO_ICMP6:
        SUCCESS_OR_EXIT(error = ip6_icmp_update_checksum(&ip6->icmp, message, checksum));
        break;
    default:
        break;
    }

    if (ip6_addr_is_multicast_larger_than_realm_local(ip6_message_info_get_peer_addr(message_info))) {
        // TODO:
    }

exit:
    if (error == NS_ERROR_NONE) {
        message_set_interface_id(message, ip6_message_info_get_interface_id(message_info));
        ip6_enqueue_datagram(ip6, message);
    }

    return error;
}

ns_error_t
ip6_send_raw(ip6_t *ip6, message_t message, int8_t interface_id)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_header_t header;
    ip6_message_info_t message_info;
    ip6_message_info_ctor(&message_info);
    bool freed = false;

    SUCCESS_OR_EXIT(error = ip6_header_init_from_message(&header, message));

    ip6_message_info_set_peer_addr(&message_info, *(ip6_addr_t *)ip6_header_get_source(&header));
    ip6_message_info_set_sock_addr(&message_info, *(ip6_addr_t *)ip6_header_get_destination(&header));
    ip6_message_info_set_interface_id(&message_info, interface_id);
    ip6_message_info_set_hop_limit(&message_info, ip6_header_get_hop_limit(&header));
    ip6_message_info_set_link_info(&message_info, NULL);

    if (ip6_addr_is_multicast(ip6_header_get_destination(&header))) {
        SUCCESS_OR_EXIT(error = ip6_insert_mpl_option(ip6, message, &header, &message_info));
    }

    error = ip6_handle_datagram(ip6, message, NULL, interface_id, NULL, true);
    freed = true;

exit:
    if (!freed) {
        message_free(message);
    }
    return error;
}

ns_error_t
ip6_handle_datagram(ip6_t *ip6,
                    message_t message,
                    ip6_netif_t *netif,
                    int8_t interface_id,
                    const void *link_message_info,
                    bool from_ncp_host)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_message_info_t message_info;
    ip6_header_t header;
    bool receive = false;
    bool forward = false;
    bool tunnel = false;
    bool multicast_promiscuous = false;
    uint8_t next_header;
    uint8_t hop_limit;
    int8_t forward_interface_id;

    SUCCESS_OR_EXIT(error = ip6_header_init_from_message(&header, message));

    ip6_message_info_set_peer_addr(&message_info, *(ip6_addr_t *)ip6_header_get_source(&header));
    ip6_message_info_set_sock_addr(&message_info, *(ip6_addr_t *)ip6_header_get_destination(&header));
    ip6_message_info_set_interface_id(&message_info, interface_id);
    ip6_message_info_set_hop_limit(&message_info, ip6_header_get_hop_limit(&header));
    ip6_message_info_set_link_info(&message_info, link_message_info);

    // determine destination of packet
    if (ip6_addr_is_multicast(ip6_header_get_destination(&header))) {
        if (netif != NULL) {
            if (ip6_netif_is_multicast_subscribed(netif, ip6_header_get_destination(&header))) {
                receive = true;
            } else if (ip6_netif_is_multicast_promiscuous_enabled(netif)) {
                multicast_promiscuous = true;
            }

            // TODO:

        } else {
            forward = true;
        }
    } else {
        if (ip6_is_unicast_addr(ip6, ip6_header_get_destination(&header))) {
            receive = true;
        } else if (!ip6_addr_is_link_local(ip6_header_get_destination(&header))) {
            forward = true;
        } else if (netif == NULL) {
            forward = true;
        }
    }

    message_set_interface_id(message, interface_id);
    message_set_offset(message, sizeof(header));

    // process IPv6 extension headers
    next_header = (uint8_t)ip6_header_get_next_header(&header);
    SUCCESS_OR_EXIT(error = ip6_handle_extension_headers(ip6, message, &header, &next_header, forward, receive));

    // process IPv6 payload
    if (receive) {
        if (next_header == IP6_IP_PROTO_IP6) {
            // remove encapsulating header
            message_remove_header(message, message_get_offset(message));
            ip6_handle_datagram(ip6, message, netif, interface_id, link_message_info, from_ncp_host);
            EXIT_NOW(tunnel = true);
        }
        ip6_process_receive_callback(ip6, message, &message_info, next_header, from_ncp_host);
        SUCCESS_OR_EXIT(error = ip6_handle_payload(ip6, message, &message_info, next_header));
    } else if (multicast_promiscuous) {
        ip6_process_receive_callback(ip6, message, &message_info, next_header, from_ncp_host);
    }

    if (forward) {
        forward_interface_id = ip6_find_forward_interface_id(ip6, &message_info);
        if (forward_interface_id == 0) {
            // try passing to host
            SUCCESS_OR_EXIT(error = ip6_process_receive_callback(ip6, message, &message_info, next_header, from_ncp_host));
            // the caller transfers custody in the success case, so free the message here
            message_free(message);
            EXIT_NOW();
        }
        if (netif != NULL) {
            VERIFY_OR_EXIT(ip6->forwarding_enabled, forward = false);
            ip6_header_set_hop_limit(&header, ip6_header_get_hop_limit(&header) - 1);
        }
        if (ip6_header_get_hop_limit(&header) == 0) {
            // send time exceeded
            EXIT_NOW(error = NS_ERROR_DROP);
        } else {
            hop_limit = ip6_header_get_hop_limit(&header);
            message_write(message,
                          ip6_header_get_hop_limit_offset(),
                          &hop_limit,
                          ip6_header_get_hop_limit_size());
            // submit message to interface
            VERIFY_OR_EXIT((netif = ip6_get_netif_by_id(ip6, forward_interface_id)) != NULL,
                           error = NS_ERROR_NO_ROUTE);
            SUCCESS_OR_EXIT(error = ip6_netif_send_message(netif, message));
        }
    }

exit:
    if (!tunnel && (error != NS_ERROR_NONE || !forward)) {
        message_free(message);
    }
    return error;
}

void
ip6_enqueue_datagram(ip6_t *ip6, message_t message)
{
    message_priority_queue_enqueue(&ip6->send_queue, message);
    tasklet_post(&ip6->send_queue_task);
}

uint16_t
ip6_update_checksum(uint16_t checksum, const ip6_addr_t *addr)
{
    return message_update_checksum_from_buffer(checksum, addr->fields.m8, sizeof(*addr));
}

uint16_t
ip6_compute_pseudo_header_checksum(const ip6_addr_t *source,
                                   const ip6_addr_t *destination,
                                   uint16_t length,
                                   ip6_ip_proto_t proto)
{
    uint16_t checksum;
    checksum = message_update_checksum_from_value(0, length);
    checksum = message_update_checksum_from_value(checksum, (uint16_t)proto);
    checksum = ip6_update_checksum(checksum, source);
    checksum = ip6_update_checksum(checksum, destination);
    return checksum;
}

void
ip6_set_receive_datagram_callback(ip6_t *ip6, ns_ip6_receive_callback_func_t callback, void *callback_context)
{
    ip6->receive_ip6_datagram_callback = callback;
    ip6->receive_ip6_datagram_callback_context = callback_context;
}

bool
ip6_is_receive_ip6_filter_enabled(ip6_t *ip6)
{
    return ip6->is_receive_ip6_filter_enabled;
}

void
ip6_set_receive_ip6_filter_enabled(ip6_t *ip6, bool enabled)
{
    ip6->is_receive_ip6_filter_enabled = enabled;
}

bool
ip6_is_forwarding_enabled(ip6_t *ip6)
{
    return ip6->forwarding_enabled;
}

void
ip6_set_forwarding_enabled(ip6_t *ip6, bool enabled)
{
    ip6->forwarding_enabled = enabled;
}

ns_error_t
ip6_add_netif(ip6_t *ip6, ip6_netif_t *anetif)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_netif_t *netif;
    if (ip6->netif_list_head == NULL) {
        ip6->netif_list_head = anetif;
    } else {
        netif = ip6->netif_list_head;
        do {
            if (netif == anetif || netif->interface_id == anetif->interface_id) {
                EXIT_NOW(error = NS_ERROR_ALREADY);
            }
        } while (netif->next);
        netif->next = anetif;
    }
    anetif->next = NULL;
exit:
    return error;
}

ns_error_t
ip6_remove_netif(ip6_t *ip6, ip6_netif_t *anetif)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    VERIFY_OR_EXIT(ip6->netif_list_head != NULL, error = NS_ERROR_NOT_FOUND);
    if (ip6->netif_list_head == anetif) {
        ip6->netif_list_head = anetif->next;
    } else {
        for (ip6_netif_t *netif = ip6->netif_list_head; netif->next; netif = netif->next) {
            if (netif->next != anetif) {
                continue;
            }
            netif->next = anetif->next;
            error = NS_ERROR_NONE;
            break;
        }
    }
    anetif->next = NULL;
exit:
    return error;
}

ip6_netif_t *
ip6_get_netif_list(ip6_t *ip6)
{
    return ip6->netif_list_head;
}

ip6_netif_t *
ip6_get_netif_by_id(ip6_t *ip6, int8_t interface_id)
{
    ip6_netif_t *netif;
    for (netif = ip6->netif_list_head; netif; netif = netif->next) {
        if (ip6_netif_get_interface_id(netif) == interface_id) {
            EXIT_NOW();
        }
    }
exit:
    return netif;
}

bool
ip6_is_unicast_addr(ip6_t *ip6, const ip6_addr_t *addr)
{
    bool rval = false;
    for (ip6_netif_t *netif = ip6->netif_list_head; netif; netif = netif->next) {
        rval = ip6_netif_is_unicast_addr(netif, addr);
        if (rval) {
            EXIT_NOW();
        }
    }
exit:
    return rval;
}

ip6_netif_unicast_addr_t *
ip6_select_source_addr(ip6_t *ip6, ip6_message_info_t *message_info)
{
    ip6_addr_t *destination = ip6_message_info_get_peer_addr(message_info);
    int interface_id = ip6_message_info_get_interface_id(message_info);
    ip6_netif_unicast_addr_t *rval_addr = NULL;
    ip6_addr_t *candidate_addr;
    int8_t candidate_id;
    int8_t rval_iface = 0;
    uint16_t rval_prefix_matched = 0;
    uint16_t destination_scope = ip6_addr_get_scope(destination);

    for (ip6_netif_t *netif = ip6_get_netif_list(ip6); netif; netif = netif->next) {
        candidate_id = ip6_netif_get_interface_id(netif);
        if (ip6_addr_is_link_local(destination) || ip6_addr_is_multicast(destination)) {
            if (interface_id != candidate_id) {
                continue;
            }
        }
        for (ip6_netif_unicast_addr_t *addr = netif->unicast_addrs; addr;
             addr = ip6_netif_unicast_addr_get_next(addr)) {
            uint8_t override_scope;
            uint8_t candidate_prefix_matched;
            candidate_addr = ip6_netif_unicast_addr_get_addr(addr);
            candidate_prefix_matched = ip6_addr_prefix_match(destination, candidate_addr);
            override_scope = (candidate_prefix_matched >= addr->prefix_length) ?
                             ip6_netif_unicast_addr_get_scope(addr) : destination_scope;
            if (ip6_addr_is_anycast_routing_locator(candidate_addr)) {
                // don't use anycast address as source address
                continue;
            }
            if (rval_addr == NULL) {
                // rule 0: prefer any adress
                rval_addr = addr;
                rval_iface = candidate_id;
                rval_prefix_matched = candidate_prefix_matched;
            } else if (ip6_addr_is_equal(candidate_addr, destination)) {
                // rule 1: prefer same address
                rval_addr = addr;
                rval_iface = candidate_id;
                EXIT_NOW();
            } else if (ip6_netif_unicast_addr_get_scope(addr) < ip6_netif_unicast_addr_get_scope(rval_addr)) {
                // rule 2: prefer appropriate scope
                if (ip6_netif_unicast_addr_get_scope(addr) >= override_scope) {
                    rval_addr = addr;
                    rval_iface = candidate_id;
                    rval_prefix_matched = candidate_prefix_matched;
                }
            } else if (ip6_netif_unicast_addr_get_scope(addr) > ip6_netif_unicast_addr_get_scope(rval_addr)) {
                if (ip6_netif_unicast_addr_get_scope(rval_addr) < override_scope) {
                    rval_addr = addr;
                    rval_iface = candidate_id;
                    rval_prefix_matched = candidate_prefix_matched;
                }
            } else if (ip6_netif_unicast_addr_get_scope(rval_addr) == IP6_ADDR_REALM_LOCAL_SCOPE) {
                // additional rule: prefer appropriate realm local address
                if (override_scope > IP6_ADDR_REALM_LOCAL_SCOPE) {
                    if (ip6_addr_is_routing_locator(ip6_netif_unicast_addr_get_addr(rval_addr))) {
                        // prefer EID if destination is not realm local
                        rval_addr = addr;
                        rval_iface = candidate_id;
                        rval_prefix_matched = candidate_prefix_matched;
                    }
                } else {
                    if (ip6_addr_is_routing_locator(candidate_addr)) {
                        // prefer RLOC if destination is realm local
                        rval_addr = addr;
                        rval_iface = candidate_id;
                        rval_prefix_matched = candidate_prefix_matched;
                    }
                }
            } else if (addr->preferred && !rval_addr->preferred) {
                // rule 3: avoid deprecated addresses
                rval_addr = addr;
                rval_iface = candidate_id;
                rval_prefix_matched = candidate_prefix_matched;
            } else if (ip6_message_info_get_interface_id(message_info) != 0 &&
                       ip6_message_info_get_interface_id(message_info) == candidate_id &&
                       rval_iface != candidate_id) {
                // rule 4: prefer home address
                // rule 5: prefer outgoing interface
                rval_addr = addr;
                rval_iface = candidate_id;
                rval_prefix_matched = candidate_prefix_matched;
            } else if (candidate_prefix_matched > rval_prefix_matched) {
                // rule 6: prefer matching level
                // rule 7: prefer public address
                // rule 8: use longest prefix matching
                rval_addr = addr;
                rval_iface = candidate_id;
                rval_prefix_matched = candidate_prefix_matched;
            }
        }
    }

exit:
    message_info->interface_id = rval_iface;
    return rval_addr;
}

int8_t
ip6_get_on_link_netif(ip6_t *ip6, const ip6_addr_t *addr)
{
    int8_t rval = -1;

    for (ip6_netif_t *netif = ip6->netif_list_head; netif; netif = netif->next) {
        for (ip6_netif_unicast_addr_t *cur = netif->unicast_addrs; cur;
             cur = ip6_netif_unicast_addr_get_next(cur)) {
            if (ip6_addr_prefix_match(ip6_netif_unicast_addr_get_addr(cur), addr) >= cur->prefix_length) {
                EXIT_NOW(rval = ip6_netif_get_interface_id(netif));
            }
        }
    }

exit:
    return rval;
}

priority_queue_t *
ip6_get_send_queue(ip6_t *ip6)
{
    return &ip6->send_queue;
}

ip6_routes_t *
ip6_get_routes(ip6_t *ip6)
{
    return &ip6->routes;
}

ip6_icmp_t *
ip6_get_icmp(ip6_t *ip6)
{
    return &ip6->icmp;
}

ip6_udp_t *
ip6_get_udp(ip6_t *ip6)
{
    return &ip6->udp;
}

ip6_mpl_t *
ip6_get_mpl(ip6_t *ip6)
{
    return &ip6->mpl;
}

const char *
ip6_ip_proto_to_string(ip6_ip_proto_t ip_proto)
{
    const char *retval;

    switch (ip_proto) {
    case IP6_IP_PROTO_HOP_OPTS:
        retval = "hop-opts";
        break;
    case IP6_IP_PROTO_TCP:
        retval = "tcp";
        break;
    case IP6_IP_PROTO_UDP:
        retval = "udp";
        break;
    case IP6_IP_PROTO_IP6:
        retval = "ip6";
        break;
    case IP6_IP_PROTO_ROUTING:
        retval = "routing";
        break;
    case IP6_IP_PROTO_FRAGMENT:
        retval = "frag";
        break;
    case IP6_IP_PROTO_ICMP6:
        retval = "icmp6";
        break;
    case IP6_IP_PROTO_NONE:
        retval = "none";
        break;
    case IP6_IP_PROTO_DST_OPTS:
        retval = "dst-opts";
        break;
    default:
        retval = "unknown";
        break;
    }

    return retval;
}

// --- private functions
static void
handle_send_queue(void *tasklet)
{
    ip6_t *ip6 = (ip6_t *)((tasklet_t *)tasklet)->handler.context;
    ip6_handle_send_queue(ip6);
}

static void
ip6_handle_send_queue(ip6_t *ip6)
{
    message_t message;
    while ((message = message_priority_queue_get_head(&ip6->send_queue)) != NULL) {
        message_priority_queue_dequeue(&ip6->send_queue, message);
        ip6_handle_datagram(ip6, message, NULL, message_get_interface_id(message), NULL, false);
    }
}

static ns_error_t
ip6_process_receive_callback(ip6_t *ip6,
                             message_t message,
                             ip6_message_info_t *message_info,
                             uint8_t ip_proto,
                             bool from_ncp_host)
{
    ns_error_t error = NS_ERROR_NONE;
    message_t message_copy = NULL;

    VERIFY_OR_EXIT(from_ncp_host == false, error = NS_ERROR_DROP);
    VERIFY_OR_EXIT(ip6->receive_ip6_datagram_callback != NULL, error = NS_ERROR_NO_ROUTE);

    if (ip6->is_receive_ip6_filter_enabled) {
        // do not pass messages sent to an RLOC/ALOC
        VERIFY_OR_EXIT(!ip6_addr_is_routing_locator(ip6_message_info_get_sock_addr(message_info)) &&
                       !ip6_addr_is_anycast_routing_locator(ip6_message_info_get_sock_addr(message_info)),
                       error = NS_ERROR_NO_ROUTE);

        switch (ip_proto) {
        case IP6_IP_PROTO_ICMP6:
            if (ip6_icmp_should_handle_echo_request(&ip6->icmp, message_info)) {
                ip6_icmp_header_t icmp;
                message_read(message, message_get_offset(message), &icmp, sizeof(icmp));
                // do not pass ICMP echo request messages
                VERIFY_OR_EXIT(ip6_icmp_header_get_type(&icmp) != IP6_ICMP_HEADER_TYPE_ECHO_REQUEST,
                               error = NS_ERROR_NO_ROUTE);
            }
            break;
        case IP6_IP_PROTO_UDP:
        {
            // TODO:
            break;
        }
        default:
            break;
        }
    }

    // make copy of the datagram to pass to host
    VERIFY_OR_EXIT((message_copy = message_clone(message)) != NULL, error = NS_ERROR_NO_BUFS);
    ip6_remove_mpl_option(ip6, message_copy);
    ip6->receive_ip6_datagram_callback(message_copy, ip6->receive_ip6_datagram_callback_context);

exit:
    switch (error) {
    case NS_ERROR_NO_BUFS:
        ns_log_warn_ip6("failed to pass up message (len: %d) to host - out of message buffer.",
                        message_get_length(message));
        break;
    case NS_ERROR_DROP:
        ns_log_note_ip6("dropping message (len: %d) from local host since next hop is the host.",
                        message_get_length(message));
        break;
    default:
        break;
    }
    return error;
}

static ns_error_t
ip6_handle_extension_headers(ip6_t *ip6,
                             message_t message,
                             ip6_header_t *header,
                             uint8_t *next_header,
                             bool forward,
                             bool receive)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_extension_header_t ext_header;

    while (receive == true || *next_header == IP6_IP_PROTO_HOP_OPTS) {
        VERIFY_OR_EXIT(message_read(message,
                                    message_get_offset(message),
                                    &ext_header,
                                    sizeof(ext_header)) == sizeof(ext_header),
                       error = NS_ERROR_DROP);
        switch (*next_header) {
        case IP6_IP_PROTO_HOP_OPTS:
            SUCCESS_OR_EXIT(error = ip6_handle_options(ip6, message, header, forward));
            break;
        case IP6_IP_PROTO_FRAGMENT:
            SUCCESS_OR_EXIT(error = ip6_handle_fragment(ip6, message));
            break;
        case IP6_IP_PROTO_DST_OPTS:
            SUCCESS_OR_EXIT(error = ip6_handle_options(ip6, message, header, forward));
            break;
        case IP6_IP_PROTO_IP6:
            EXIT_NOW();
        case IP6_IP_PROTO_ROUTING:
        case IP6_IP_PROTO_NONE:
            EXIT_NOW(error = NS_ERROR_DROP);
        default:
            EXIT_NOW();
        }

        *next_header = (uint8_t)ip6_extension_header_get_next_header(&ext_header);
    }

exit:
    return error;
}

static ns_error_t
ip6_handle_fragment(ip6_t *ip6, message_t message)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_fragment_header_t fragment_header;

    VERIFY_OR_EXIT(message_read(message,
                                message_get_offset(message),
                                &fragment_header,
                                sizeof(fragment_header)) == sizeof(fragment_header),
                   error = NS_ERROR_DROP);
    VERIFY_OR_EXIT(ip6_fragment_header_get_offset(&fragment_header) == 0 &&
                   ip6_fragment_header_is_more_flag_set(&fragment_header) == false,
                   error = NS_ERROR_DROP);
    message_move_offset(message, sizeof(fragment_header));
exit:
    return error;
}

static ns_error_t
ip6_add_mpl_option(ip6_t *ip6, message_t message, ip6_header_t *header)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_hop_by_hop_header_t hbh_header;
    ip6_option_mpl_t mpl_option;
    ip6_option_padn_t pad_option;

    ip6_hop_by_hop_header_set_next_header(&hbh_header, ip6_header_get_next_header(header));
    ip6_hop_by_hop_header_set_length(&hbh_header, 0);
    ip6_mpl_init_option(&ip6->mpl, &mpl_option, ip6_header_get_source(header));

    // mpl option may require two bytes padding
    if ((ip6_option_mpl_get_total_length(&mpl_option) + sizeof(hbh_header)) % 8) {
        ip6_option_padn_init(&pad_option, 2);
        SUCCESS_OR_EXIT(error = message_prepend(message, &pad_option, ip6_option_padn_get_total_length(&pad_option)));
    }

    SUCCESS_OR_EXIT(error = message_prepend(message, &mpl_option, ip6_option_mpl_get_total_length(&mpl_option)));
    SUCCESS_OR_EXIT(error = message_prepend(message, &hbh_header, sizeof(hbh_header)));

    ip6_header_set_payload_length(header,
                                  ip6_header_get_payload_length(header) +
                                  sizeof(hbh_header) +
                                  sizeof(mpl_option));

    ip6_header_set_next_header(header, IP6_IP_PROTO_HOP_OPTS);

exit:
    return error;
}

static ns_error_t
ip6_add_tunneled_mpl_option(ip6_t *ip6,
                            message_t message,
                            ip6_header_t *header,
                            ip6_message_info_t *message_info)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_header_t tunnel_header;
    ip6_netif_unicast_addr_t *source;
    ip6_message_info_t msg_info = *message_info;

    // Use IP-in-IP encapsulation (RFC2473) and ALL_MPL_FORWARDERS address.
    memset(ip6_message_info_get_peer_addr(&msg_info), 0, sizeof(ip6_addr_t));
    ((ip6_addr_t *)ip6_message_info_get_peer_addr(&msg_info))->fields.m16[0] = encoding_swap16(0xff03);
    ((ip6_addr_t *)ip6_message_info_get_peer_addr(&msg_info))->fields.m16[7] = encoding_swap16(0x00fc);

    ip6_header_init(&tunnel_header);
    ip6_header_set_hop_limit(&tunnel_header, (uint8_t)IP6_DEFAULT_HOP_LIMIT);
    ip6_header_set_payload_length(&tunnel_header, ip6_header_get_payload_length(header) + sizeof(tunnel_header));
    ip6_header_set_destination(&tunnel_header, ip6_message_info_get_peer_addr(&msg_info));
    ip6_header_set_next_header(&tunnel_header, IP6_IP_PROTO_IP6);

    VERIFY_OR_EXIT((source = ip6_select_source_addr(ip6, &msg_info)) != NULL, error = NS_ERROR_INVALID_SOURCE_ADDRESS);

    ip6_header_set_source(&tunnel_header, ip6_netif_unicast_addr_get_addr(source));

    SUCCESS_OR_EXIT(error = ip6_add_mpl_option(ip6, message, &tunnel_header));
    SUCCESS_OR_EXIT(error = message_prepend(message, &tunnel_header, sizeof(tunnel_header)));

exit:
    return error;
}

static ns_error_t
ip6_insert_mpl_option(ip6_t *ip6,
                      message_t message,
                      ip6_header_t *header,
                      ip6_message_info_t *message_info)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(ip6_addr_is_multicast(ip6_header_get_destination(header)) &&
                   ip6_addr_get_scope(ip6_header_get_destination(header)) >= IP6_ADDR_REALM_LOCAL_SCOPE);

    if (ip6_addr_is_realm_local_multicast(ip6_header_get_destination(header))) {
        message_remove_header(message, sizeof(*header));
        if (ip6_header_get_next_header(header) == IP6_IP_PROTO_HOP_OPTS) {
            ip6_hop_by_hop_header_t hbh;
            uint16_t hbh_length = 0;
            ip6_option_mpl_t mpl_option;

            // read existing hop-by-hop option header
            message_read(message, 0, &hbh, sizeof(hbh));
            hbh_length = (ip6_hop_by_hop_header_get_length(&hbh) + 1) * 8;

            VERIFY_OR_EXIT(hbh_length <= ip6_header_get_payload_length(header), error = NS_ERROR_PARSE);

            // increase existing hop-by-hop option header length by 8 bytes
            ip6_hop_by_hop_header_set_length(&hbh, ip6_hop_by_hop_header_get_length(&hbh) + 1);
            message_write(message, 0, &hbh, sizeof(hbh));

            // make space for MPL option + padding by shifting hop by hop option header
            SUCCESS_OR_EXIT(error = message_prepend(message, NULL, 8));
            message_copy_to(message, 8, 0, hbh_length, message);

            // insert MPL option
            ip6_mpl_init_option(&ip6->mpl, &mpl_option, ip6_header_get_source(header));
            message_write(message, hbh_length, &mpl_option, ip6_option_mpl_get_total_length(&mpl_option));

            // insert pad option if needed
            if (ip6_option_mpl_get_total_length(&mpl_option) % 8) {
                ip6_option_padn_t pad_option;
                ip6_option_padn_init(&pad_option, 8 - (ip6_option_mpl_get_total_length(&mpl_option) % 8));
                message_write(message,
                              hbh_length + ip6_option_mpl_get_total_length(&mpl_option),
                              &pad_option,
                              ip6_option_padn_get_total_length(&pad_option));
            }

            // increase IPv6 payload length
            ip6_header_set_payload_length(header, ip6_header_get_payload_length(header) + 8);
        } else {
            SUCCESS_OR_EXIT(error = ip6_add_mpl_option(ip6, message, header));
        }
        SUCCESS_OR_EXIT(error = message_prepend(message, header, sizeof(*header)));
    } else {

        // TODO:

        SUCCESS_OR_EXIT(error = ip6_add_tunneled_mpl_option(ip6, message, header, message_info));
    }

exit:
    return error;
}

static ns_error_t
ip6_remove_mpl_option(ip6_t *ip6, message_t message)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_header_t ip6_header;
    ip6_hop_by_hop_header_t hbh;
    uint16_t offset;
    uint16_t end_offset;
    uint16_t mpl_offset = 0;
    uint8_t mpl_length = 0;
    bool remove = false;

    offset = 0;
    message_read(message, offset, &ip6_header, sizeof(ip6_header));
    offset += sizeof(ip6_header);
    VERIFY_OR_EXIT(ip6_header_get_next_header(&ip6_header) == IP6_IP_PROTO_HOP_OPTS);

    message_read(message, offset, &hbh, sizeof(hbh));
    end_offset = offset + (ip6_hop_by_hop_header_get_length(&hbh) + 1) * 8;
    VERIFY_OR_EXIT(message_get_length(message) >= end_offset, error = NS_ERROR_PARSE);

    offset += sizeof(hbh);

    while (offset < end_offset) {
        ip6_option_header_t option;
        ip6_option_header_ctor(&option);

        message_read(message, offset, &option, sizeof(option));

        switch (ip6_option_header_get_type(&option)) {
        case IP6_OPTION_MPL_TYPE:
            mpl_offset = offset;
            mpl_length = ip6_option_header_get_length(&option);
            if (mpl_offset == sizeof(ip6_header) + sizeof(hbh) &&
                ip6_hop_by_hop_header_get_length(&hbh) == 0) {
                // first and only IPv6 option, remove IPv6 HBH option header
                remove = true;
            } else if (mpl_offset + 8 == end_offset) {
                // last IPv6 option, remove last 8 bytes
                remove = true;
            }
            offset += sizeof(option) + ip6_option_header_get_length(&option);
            break;
        case IP6_OPTION_PAD1_TYPE:
            offset += sizeof(ip6_option_padn_t);
            break;
        case IP6_OPTION_PADN_TYPE:
            offset += sizeof(option) + ip6_option_header_get_length(&option);
            break;
        default:
            // encountered another option, now just replace MPL option with PADN
            remove = false;
            offset += sizeof(option) + ip6_option_header_get_length(&option);
            break;
        }
    }

    // verify that IPv6 options header is properly formed
    if (remove) {
        // last IPv6 option, shrink HBH option header
        uint8_t buf[8];

        offset = end_offset - sizeof(buf);

        while (offset >= sizeof(buf)) {
            message_read(message, offset - sizeof(buf), buf, sizeof(buf));
            message_write(message, offset, buf, sizeof(buf));
            offset -= sizeof(buf);
        }

        message_remove_header(message, sizeof(buf));

        if (mpl_offset == sizeof(ip6_header) + sizeof(hbh)) {
            // remove entire HBH header
            ip6_header_set_next_header(&ip6_header, ip6_hop_by_hop_header_get_next_header(&hbh));
        } else {
            // update HBH header length
            ip6_hop_by_hop_header_set_length(&hbh, ip6_hop_by_hop_header_get_length(&hbh) - 1);
            message_write(message, sizeof(ip6_header), &hbh, sizeof(hbh));
        }

        ip6_header_set_payload_length(&ip6_header, ip6_header_get_payload_length(&ip6_header) - sizeof(buf));
        message_write(message, 0, &ip6_header, sizeof(ip6_header));
    } else if (mpl_offset != 0) {
        // replace MPL option with padn option
        ip6_option_padn_t pad_option;
        ip6_option_padn_init(&pad_option, sizeof(ip6_option_header_t) + mpl_length);
        message_write(message, mpl_offset, &pad_option, ip6_option_padn_get_total_length(&pad_option));
    }

exit:
    return error;
}

static ns_error_t
ip6_handle_options(ip6_t *ip6, message_t message, ip6_header_t *header, bool forward)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_hop_by_hop_header_t hbh_header;
    ip6_option_header_t option_header;
    ip6_option_header_ctor(&option_header);
    uint16_t end_offset;

    VERIFY_OR_EXIT(message_read(message,
                                message_get_offset(message),
                                &hbh_header,
                                sizeof(hbh_header)) == sizeof(hbh_header),
                   error = NS_ERROR_DROP);
    end_offset = message_get_offset(message) + (ip6_hop_by_hop_header_get_length(&hbh_header) + 1) * 8;

    VERIFY_OR_EXIT(end_offset <= message_get_length(message), error = NS_ERROR_DROP);

    message_move_offset(message, sizeof(option_header));

    while (message_get_offset(message) < end_offset) {
        VERIFY_OR_EXIT(message_read(message,
                                    message_get_offset(message),
                                    &option_header,
                                    sizeof(option_header)) == sizeof(option_header),
                       error = NS_ERROR_DROP); 

        if (ip6_option_header_get_type(&option_header) == IP6_OPTION_PAD1_TYPE) {
            message_move_offset(message, sizeof(ip6_option_pad1_t));
            continue;
        }

        VERIFY_OR_EXIT(message_get_offset(message) +
                       sizeof(option_header) +
                       ip6_option_header_get_length(&option_header) <= end_offset,
                       error = NS_ERROR_DROP);

        switch (ip6_option_header_get_type(&option_header)) {
        case IP6_OPTION_MPL_TYPE:
            SUCCESS_OR_EXIT(error = ip6_mpl_process_option(&ip6->mpl,
                                                           message,
                                                           ip6_header_get_source(header),
                                                           forward));
            break;
        default:
            switch (ip6_option_header_get_action(&option_header)) {
            case IP6_OPTION_HEADER_ACTION_SKIP:
                break;
            case IP6_OPTION_HEADER_ACTION_DISCARD:
                EXIT_NOW(error = NS_ERROR_DROP);
            case IP6_OPTION_HEADER_ACTION_FORCE_ICMP:
                // TODO: send icmp error
                EXIT_NOW(error = NS_ERROR_DROP);
            case IP6_OPTION_HEADER_ACTION_ICMP:
                // TODO: send icmp error
                EXIT_NOW(error = NS_ERROR_DROP);
            }
            break;
        }

        message_move_offset(message, sizeof(option_header) + ip6_option_header_get_length(&option_header));
    }

exit:
    return error;
}

static ns_error_t
ip6_handle_payload(ip6_t *ip6, message_t message, ip6_message_info_t *message_info, uint8_t ip_proto)
{
    ns_error_t error = NS_ERROR_NONE;
    switch (ip_proto) {
    case IP6_IP_PROTO_UDP:
        EXIT_NOW(error = ip6_udp_handle_message(&ip6->udp, message, message_info));
    case IP6_IP_PROTO_ICMP6:
        EXIT_NOW(error = ip6_icmp_handle_message(&ip6->icmp, message, message_info));
    }
exit:
    return error;
}

static int8_t
ip6_find_forward_interface_id(ip6_t *ip6, ip6_message_info_t *message_info)
{
    int8_t interface_id;

    if (ip6_addr_is_multicast(ip6_message_info_get_sock_addr(message_info))) {
        // multicast
        interface_id = message_info->interface_id;
    } else if (ip6_addr_is_link_local(ip6_message_info_get_sock_addr(message_info))) {
        // on-link link-local address
        interface_id = message_info->interface_id;
    } else if ((interface_id = ip6_get_on_link_netif(ip6, ip6_message_info_get_sock_addr(message_info))) > 0) {
        // on-link global address
        ;
    } else if ((interface_id = ip6_routes_lookup(&ip6->routes,
                                                 ip6_message_info_get_peer_addr(message_info),
                                                 ip6_message_info_get_sock_addr(message_info))) > 0) {
        // route
        ;
    } else {
        interface_id = 0;
    }

    return interface_id;
}
