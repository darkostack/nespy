#ifndef NS_IP6_H_
#define NS_IP6_H_

#include "ns/message.h"
#include "ns/platform/radio.h"

#define NS_IP6_PREFIX_SIZE 8 // size of an IPv6 prefix (bytes)
#define NS_IP6_IID_SIZE 8 // size of an IPv6 interface identifier (bytes)
#define NS_IP6_ADDRESS_SIZE 16 // size of an IPv6 address (bytes)

typedef struct _ns_ip6_addr ns_ip6_addr_t;
struct _ns_ip6_addr {
    union {
        uint8_t m8[NS_IP6_ADDRESS_SIZE];
        uint16_t m16[NS_IP6_ADDRESS_SIZE / sizeof(uint16_t)];
        uint32_t m32[NS_IP6_ADDRESS_SIZE / sizeof(uint32_t)];
    } fields;
};

typedef struct _ns_ip6_prefix ns_ip6_prefix_t;
struct _ns_ip6_prefix {
    ns_ip6_addr_t prefix;
    uint8_t length;
};


typedef struct _ns_netif_addr ns_netif_addr_t;
struct _ns_netif_addr {
    ns_ip6_addr_t addr;  // IPv6 unicast address
    uint8_t prefix_length;
    bool preferred : 1;
    bool valid: 1;
    bool scope_override_valid : 1;
    unsigned int scope_override : 4;
    bool rloc : 1;
    ns_netif_addr_t *next;
};

typedef struct _ns_netif_multicast_addr ns_netif_multicast_addr_t;
struct _ns_netif_multicast_addr {
    ns_ip6_addr_t addr; // IPv6 multicast address
    ns_netif_multicast_addr_t *next;
};

typedef enum _ns_netif_interface_id {
    NS_NETIF_INTERFACE_ID_HOST   = -1, // The interface ID telling packets received by host side interface
    NS_NETIF_INTERFACE_ID_THREAD = 1,  // The Thread Network interface ID
} ns_netif_interface_id_t;

typedef struct _ns_semantically_opaque_iid_generator_data {
    uint8_t *interface_id;
    uint8_t interface_id_length;
    uint8_t *network_id;
    uint8_t network_id_length;
    uint8_t dad_counter;
    uint8_t *secret_key;
    uint16_t secret_key_length;
} ns_semantically_opaque_iid_generator_data_t;

typedef struct _ns_sock_addr {
    ns_ip6_addr_t addr;
    uint16_t port;
    int8_t scope_id;
} ns_sock_addr_t;

typedef struct _ns_message_info {
    ns_ip6_addr_t sock_addr;
    ns_ip6_addr_t peer_addr;
    uint16_t sock_port;
    uint16_t peer_port;
    int8_t interface_id;
    uint8_t hop_limit;
    const void *link_info;
} ns_message_info_t;

ns_error_t
ns_ip6_set_enabled(ns_instance_t instance, bool enabled);

bool
ns_ip6_is_enabled(ns_instance_t instance);

ns_error_t
ns_ip6_add_unicast_addr(ns_instance_t instance, const ns_netif_addr_t *addr);

ns_error_t
ns_ip6_remove_unicast_addr(ns_instance_t instance, const ns_ip6_addr_t *addr);

const ns_netif_addr_t *
ns_ip6_get_unicast_addrs(ns_instance_t instance);

ns_error_t
ns_ip6_subscribe_multicast_addr(ns_instance_t instance, const ns_ip6_addr_t *addr);

ns_error_t
ns_ip6_unsubscribe_multicast_addr(ns_instance_t instance, const ns_ip6_addr_t *addr);

const ns_netif_multicast_addr_t *
ns_ip6_get_multicast_addrs(ns_instance_t instance);

bool
ns_ip6_is_multicast_promiscuous_enabled(ns_instance_t instance);

void
ns_ip6_set_multicast_promiscuous_enabled(ns_instance_t instance, bool enabled);

typedef ns_error_t (*ns_ip6_slaac_iid_create_func_t)(ns_instance_t instance, ns_netif_addr_t *addr, void *context);

void
ns_ip6_slaac_update(ns_instance_t instance,
                    ns_netif_addr_t *addrs,
                    uint32_t num_addrs,
                    ns_ip6_slaac_iid_create_func_t iid_create,
                    void *context);

ns_error_t
ns_ip6_create_random_iid(ns_instance_t instance, ns_netif_addr_t *addrs, void *context);

ns_error_t
ns_ip6_create_mac_iid(ns_instance_t instance, ns_netif_addr_t *addrs, void *context);

ns_error_t
ns_ip6_create_semantically_opaque_iid(ns_instance_t instance, ns_netif_addr_t *addrs, void *context);

ns_message_t
ns_ip6_new_message(ns_instance_t instance, const ns_message_settings_t *settings);

typedef void (*ns_ip6_receive_callback_func_t)(ns_message_t message, void *context);

void
ns_ip6_set_receive_callback(ns_instance_t instance, ns_ip6_receive_callback_func_t callback, void *context);

typedef void (*ns_ip6_addr_callback_func_t)(const ns_ip6_addr_t *addr,
                                            uint8_t prefix_length,
                                            bool is_added,
                                            void *context);

void
ns_ip6_set_addr_callback(ns_instance_t instance, ns_ip6_addr_callback_func_t callback, void *context);

bool
ns_ip6_is_receive_filter_enabled(ns_instance_t instance);

void
ns_ip6_set_receive_filter_enabled(ns_instance_t instance, bool enabled);

ns_error_t
ns_ip6_send(ns_instance_t instance, ns_message_t message);

ns_error_t
ns_ip6_add_unsecure_port(ns_instance_t instance, uint16_t port);

ns_error_t
ns_ip6_remove_unsecure_port(ns_instance_t instance, uint16_t port);

void
ns_ip6_remove_all_unsecure_ports(ns_instance_t instance);

const uint16_t *
ns_ip6_get_unsecure_ports(ns_instance_t instance, uint8_t *num_entries);

bool
ns_ip6_is_addr_equal(ns_ip6_addr_t *first, ns_ip6_addr_t *second);

ns_error_t
ns_ip6_addr_from_string(const char *string, ns_ip6_addr_t *addr);

uint8_t
ns_ip6_prefix_match(const ns_ip6_addr_t *first, const ns_ip6_addr_t *second);

bool
ns_ip6_is_addr_unspecified(const ns_ip6_addr_t *addr);

ns_error_t
ns_ip6_select_source_addr(ns_instance_t instance, ns_message_info_t *message_info);

#endif // NS_IP6_H_
