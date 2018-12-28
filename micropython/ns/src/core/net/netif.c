#include "core/net/netif.h"
#include "core/common/instance.h"
#include "core/common/code_utils.h"
#include "core/common/debug.h"

// "ff03::fc"
static const ns_netif_multicast_addr_t s_ip6_netif_realm_local_all_mpl_forwarders_multicast_addr = {
    {{{0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc}}},
    NULL};

// "ff03::01"
static const ns_netif_multicast_addr_t s_ip6_netif_realm_local_all_nodes_muticast_addr = {
    {{{0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}}},
    &s_ip6_netif_realm_local_all_mpl_forwarders_multicast_addr};

// "ff02::01"
static const ns_netif_multicast_addr_t s_ip6_netif_link_local_all_nodes_multicast_addr = {
    {{{0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}}},
    &s_ip6_netif_realm_local_all_nodes_muticast_addr};

// "ff03:02"
static const ns_netif_multicast_addr_t s_ip6_netif_realm_local_all_routers_multicast_addr = {
    {{{0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}}},
    &s_ip6_netif_link_local_all_nodes_multicast_addr};

// "ff02:02"
static const ns_netif_multicast_addr_t s_ip6_netif_link_local_all_routers_multicast_addr = {
    {{{0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}}},
    &s_ip6_netif_realm_local_all_routers_multicast_addr};

// --- Ip6 Netif unicast address
ip6_addr_t *
ip6_netif_unicast_addr_get_addr(ip6_netif_unicast_addr_t *netif_unicast_addr)
{
    return (ip6_addr_t *)&netif_unicast_addr->addr;
}

uint8_t
ip6_netif_unicast_addr_get_scope(ip6_netif_unicast_addr_t *netif_unicast_addr)
{
    return netif_unicast_addr->scope_override_valid ? (uint8_t)netif_unicast_addr->scope_override : ip6_addr_get_scope(&netif_unicast_addr->addr);
}

ip6_netif_unicast_addr_t *
ip6_netif_unicast_addr_get_next(ip6_netif_unicast_addr_t *netif_unicast_addr)
{
    return (ip6_netif_unicast_addr_t *)netif_unicast_addr->next;
}

// --- Ip6 Netif multicast address
ip6_addr_t *
ip6_netif_multicast_addr_get_addr(ip6_netif_multicast_addr_t *netif_multicast_addr)
{
    return (ip6_addr_t *)&netif_multicast_addr->addr;
}

ip6_netif_multicast_addr_t *
ip6_netif_multicast_addr_get_next(ip6_netif_multicast_addr_t *netif_multicast_addr)
{
    return (ip6_netif_multicast_addr_t *)netif_multicast_addr->next;
}

// --- Ip6 Netif
void
ip6_netif_ctor(ip6_netif_t *ip6_netif, void *instance, int8_t interface_id)
{
    ip6_netif->instance = instance;
    ip6_netif->unicast_addrs = NULL;
    ip6_netif->multicast_addrs = NULL;
    ip6_netif->interface_id = interface_id;
    ip6_netif->multicast_promiscuous = false;
    ip6_netif->next = NULL;
    ip6_netif->addr_callback = NULL;
    ip6_netif->addr_callback_context = NULL;

    ip6_netif->realm_local_all_mpl_forwarders_multicast_addr =
               s_ip6_netif_realm_local_all_mpl_forwarders_multicast_addr;
    ip6_netif->link_local_all_nodes_multicast_addr =
               s_ip6_netif_link_local_all_nodes_multicast_addr;
    ip6_netif->realm_local_all_nodes_multicast_addr =
               s_ip6_netif_realm_local_all_nodes_muticast_addr;
    ip6_netif->link_local_all_routers_multicast_addr =
               s_ip6_netif_link_local_all_routers_multicast_addr;

    for (size_t i = 0; i < NS_ARRAY_LENGTH(ip6_netif->ext_unicast_addrs); i++) {
        // to mark the address as unused/available, set the `next` to point back to itself
        ip6_netif->ext_unicast_addrs[i].next = &ip6_netif->ext_unicast_addrs[i];
    }

    for (size_t i = 0; i < NS_ARRAY_LENGTH(ip6_netif->ext_multicast_addrs); i++) {
        // to mark the address as unused/available, set the `next` to point back to itself
        ip6_netif->ext_multicast_addrs[i].next = &ip6_netif->ext_multicast_addrs[i];
    }
}

ip6_netif_t *
ip6_netif_get_next(ip6_netif_t *ip6_netif)
{
    return ip6_netif->next;
}

int8_t
ip6_netif_get_interface_id(ip6_netif_t *ip6_netif)
{
    return ip6_netif->interface_id;
}

void
ip6_netif_set_addr_callback(ip6_netif_t *ip6_netif, ns_ip6_addr_callback_func_t callback, void *callback_context)
{
    ip6_netif->addr_callback = callback;
    ip6_netif->addr_callback_context = callback_context;
}

ip6_netif_unicast_addr_t *
ip6_netif_get_unicast_addrs(ip6_netif_t *ip6_netif)
{
    return ip6_netif->unicast_addrs;
}

ns_error_t
ip6_netif_add_unicast_addr(ip6_netif_t *ip6_netif, ip6_netif_unicast_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;

    for (ip6_netif_unicast_addr_t *cur = ip6_netif->unicast_addrs; cur;
         cur = ip6_netif_unicast_addr_get_next(cur)) {
        if (cur == addr) {
            EXIT_NOW(error = NS_ERROR_ALREADY);
        }
    }

    addr->next = ip6_netif->unicast_addrs;
    ip6_netif->unicast_addrs = addr;

    if (ip6_netif->addr_callback != NULL) {
        ip6_netif->addr_callback(&addr->addr, addr->prefix_length, true, ip6_netif->addr_callback_context);
    }

    notifier_signal(instance_get_notifier(ip6_netif->instance), addr->rloc ? NS_CHANGED_THREAD_RLOC_ADDED : NS_CHANGED_IP6_ADDRESS_ADDED);

exit:
    return error;
}

ns_error_t
ip6_netif_remove_unicast_addr(ip6_netif_t *ip6_netif, ip6_netif_unicast_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;

    if (ip6_netif->unicast_addrs == addr) {
        ip6_netif->unicast_addrs = ip6_netif_unicast_addr_get_next(ip6_netif->unicast_addrs);
        EXIT_NOW();
    } else if (ip6_netif->unicast_addrs != NULL) {
        for (ip6_netif_unicast_addr_t *cur = ip6_netif->unicast_addrs;
             ip6_netif_unicast_addr_get_next(cur);
             cur = ip6_netif_unicast_addr_get_next(cur)) {
            if (cur->next == addr) {
                cur->next = addr->next;
                EXIT_NOW();
            }
        }
    }

    EXIT_NOW(error = NS_ERROR_NOT_FOUND);

exit:

    if (error != NS_ERROR_NOT_FOUND) {
        if (ip6_netif->addr_callback != NULL) {
            ip6_netif->addr_callback(&addr->addr, addr->prefix_length, false, ip6_netif->addr_callback_context);
        }
        notifier_signal(instance_get_notifier(ip6_netif->instance), addr->rloc ? NS_CHANGED_THREAD_RLOC_REMOVED : NS_CHANGED_IP6_ADDRESS_REMOVED);
    }

    return error;
}

ns_error_t
ip6_netif_add_external_unicast_addr(ip6_netif_t *ip6_netif, ip6_netif_unicast_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_netif_unicast_addr_t *entry;
    size_t num = NS_ARRAY_LENGTH(ip6_netif->ext_unicast_addrs);
    uint8_t i = 0;

    VERIFY_OR_EXIT(!ip6_addr_is_link_local(ip6_netif_unicast_addr_get_addr(addr)), error = NS_ERROR_INVALID_ARGS);

    for (entry = ip6_netif->unicast_addrs; entry; entry = ip6_netif_unicast_addr_get_next(entry)) {
        if (ip6_addr_is_equal(ip6_netif_unicast_addr_get_addr(entry), ip6_netif_unicast_addr_get_addr(addr))) {
            VERIFY_OR_EXIT((entry >= &ip6_netif->ext_unicast_addrs[0]) &&
                           (entry < &ip6_netif->ext_unicast_addrs[num]),
                           error = NS_ERROR_INVALID_ARGS);
            entry->prefix_length = addr->prefix_length;
            entry->preferred = addr->preferred;
            entry->valid = addr->valid;
            EXIT_NOW();
        }
    }

    // find an available entry in the `ext_unicast_addrs` array
    for (entry = &ip6_netif->ext_unicast_addrs[0]; num > 0; num--, entry++, i++) {
        // in an unused/available entry, `next` points back to the entry itself
        if (entry->next == entry) {
            break;
        }
    }

    VERIFY_OR_EXIT(num > 0, error = NS_ERROR_NO_BUFS);

    // copy the new address into the available entry and insert it in linked-list
    ip6_netif->ext_unicast_addrs[i] = *addr;
    ip6_netif->ext_unicast_addrs[i].next = ip6_netif->unicast_addrs;
    ip6_netif->unicast_addrs = &ip6_netif->ext_unicast_addrs[i];

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_ADDRESS_ADDED);

exit:
    return error;
}

ns_error_t
ip6_netif_remove_external_unicast_addr(ip6_netif_t *ip6_netif, ip6_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_netif_unicast_addr_t *entry;
    ip6_netif_unicast_addr_t *last = NULL;
    size_t num = NS_ARRAY_LENGTH(ip6_netif->ext_unicast_addrs);

    for (entry = ip6_netif->unicast_addrs; entry; entry = ip6_netif_unicast_addr_get_next(entry)) {
        if (ip6_addr_is_equal(ip6_netif_unicast_addr_get_addr(entry), addr)) {
            VERIFY_OR_EXIT((entry >= &ip6_netif->ext_unicast_addrs[0]) &&
                           (entry < &ip6_netif->ext_unicast_addrs[num]),
                           error = NS_ERROR_INVALID_ARGS);
            if (last) {
                last->next = entry->next;
            } else {
                ip6_netif->unicast_addrs = ip6_netif_unicast_addr_get_next(entry);
            }
            break;
        }
        last = entry;
    }

    VERIFY_OR_EXIT(entry != NULL, error = NS_ERROR_NOT_FOUND);

    // to mark the address entry as unused/available, set the `next` pointer back to the entry itself
    entry->next = entry;

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_ADDRESS_REMOVED);

exit:
    return error;
}

void
ip6_netif_remove_all_external_unicast_addrs(ip6_netif_t *ip6_netif)
{
    size_t num = NS_ARRAY_LENGTH(ip6_netif->ext_unicast_addrs);
    for (ip6_netif_unicast_addr_t *entry = &ip6_netif->ext_unicast_addrs[0]; num > 0; num--, entry++) {
        // in unused entries, the `next` points back to the entry itself
        if (entry->next != entry) {
            ip6_netif_remove_external_unicast_addr(ip6_netif, ip6_netif_unicast_addr_get_addr(entry));
        }
    }
}

bool
ip6_netif_is_unicast_addr(ip6_netif_t *ip6_netif, const ip6_addr_t *addr)
{
    bool rval = false;

    for (ip6_netif_unicast_addr_t *cur = ip6_netif->unicast_addrs; cur;
         cur = ip6_netif_unicast_addr_get_next(cur)) {
        if (ip6_addr_is_equal(ip6_netif_unicast_addr_get_addr(cur), addr)) {
            EXIT_NOW(rval = true);
        }
    }

exit:
    return rval;
}

bool
ip6_netif_is_multicast_subscribed(ip6_netif_t *ip6_netif, const ip6_addr_t *addr)
{
    bool rval = false;

    for (ip6_netif_multicast_addr_t *cur = ip6_netif->multicast_addrs; cur;
         cur = ip6_netif_multicast_addr_get_next(cur)) {
        if (ip6_addr_is_equal(ip6_netif_multicast_addr_get_addr(cur), addr)) {
            EXIT_NOW(rval = true);
        }
    }

exit:
    return rval;
}

ns_error_t
ip6_netif_subscribe_all_routers_multicast(ip6_netif_t *ip6_netif)
{
    ns_error_t error = NS_ERROR_NONE;

    if (ip6_netif->multicast_addrs == &ip6_netif->link_local_all_nodes_multicast_addr) {
        ip6_netif->multicast_addrs = (ip6_netif_multicast_addr_t *)((ns_netif_multicast_addr_t *)(&ip6_netif->link_local_all_routers_multicast_addr));
    } else {
        for (ip6_netif_multicast_addr_t *cur = ip6_netif->multicast_addrs; cur;
             cur = ip6_netif_multicast_addr_get_next(cur)) {
            if (cur == &ip6_netif->link_local_all_routers_multicast_addr) {
                EXIT_NOW(error = NS_ERROR_ALREADY);
            }
            if (cur->next == &ip6_netif->link_local_all_nodes_multicast_addr) {
                cur->next = &ip6_netif->link_local_all_routers_multicast_addr;
                break;
            }
        }
    }

    if (ip6_netif->addr_callback != NULL) {
        for (const ns_netif_multicast_addr_t * entry = &ip6_netif->link_local_all_routers_multicast_addr;
             entry != &ip6_netif->link_local_all_nodes_multicast_addr;
             entry = entry->next) {
            ip6_netif->addr_callback(&entry->addr, IP6_NETIF_MULTICAST_PREFIX_LENGTH, true, ip6_netif->addr_callback_context);
        }
    }

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_SUBSRCRIBED);

exit:
    return error;
}

ns_error_t
ip6_netif_unsubscribe_all_routers_multicast(ip6_netif_t *ip6_netif)
{
    ns_error_t error = NS_ERROR_NONE;

    if (ip6_netif->multicast_addrs == &ip6_netif->link_local_all_routers_multicast_addr) {
        ip6_netif->multicast_addrs = (ip6_netif_multicast_addr_t *)((ns_netif_multicast_addr_t *)(&ip6_netif->link_local_all_nodes_multicast_addr));
        EXIT_NOW();
    }

    for (ip6_netif_multicast_addr_t *cur = ip6_netif->multicast_addrs; cur;
         cur = ip6_netif_multicast_addr_get_next(cur)) {
        if (cur->next == &ip6_netif->link_local_all_routers_multicast_addr) {
            cur->next = &ip6_netif->link_local_all_nodes_multicast_addr;
            EXIT_NOW();
        }
    }

    error = NS_ERROR_NOT_FOUND;

exit:

    if (error != NS_ERROR_NOT_FOUND) {
        if (ip6_netif->addr_callback != NULL) {
            for (const ns_netif_multicast_addr_t *entry = &ip6_netif->link_local_all_routers_multicast_addr;
                 entry != &ip6_netif->link_local_all_nodes_multicast_addr;
                 entry = entry->next) {
                ip6_netif->addr_callback(&entry->addr, IP6_NETIF_MULTICAST_PREFIX_LENGTH, false, ip6_netif->addr_callback_context);
            }
        }
        notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_UNSUBSRCRIBED);
    }

    return error;
}

const ip6_netif_multicast_addr_t *
ip6_netif_get_multicast_addrs(ip6_netif_t *ip6_netif)
{
    return ip6_netif->multicast_addrs;
}

ns_error_t
ip6_netif_subscribe_multicast(ip6_netif_t *ip6_netif, ip6_netif_multicast_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;

    for (ip6_netif_multicast_addr_t *cur = ip6_netif->multicast_addrs; cur;
         cur = ip6_netif_multicast_addr_get_next(cur)) {
        if (cur == addr) {
            EXIT_NOW(error = NS_ERROR_ALREADY);
        }
    }

    addr->next = ip6_netif->multicast_addrs;
    ip6_netif->multicast_addrs = addr;

    if (ip6_netif->addr_callback != NULL) {
        ip6_netif->addr_callback(&addr->addr, IP6_NETIF_MULTICAST_PREFIX_LENGTH, true, ip6_netif->addr_callback_context);
    }

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_SUBSRCRIBED);

exit:
    return error;
}

ns_error_t
ip6_netif_unsubscribe_multicast(ip6_netif_t *ip6_netif, const ip6_netif_multicast_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;

    if (ip6_netif->multicast_addrs == addr) {
        ip6_netif->multicast_addrs = ip6_netif_multicast_addr_get_next(ip6_netif->multicast_addrs);
        EXIT_NOW();
    } else if (ip6_netif->multicast_addrs != NULL) {
        for (ip6_netif_multicast_addr_t *cur = ip6_netif->multicast_addrs;
             ip6_netif_multicast_addr_get_next(cur);
             cur = ip6_netif_multicast_addr_get_next(cur)) {
            if (cur->next == addr) {
                cur->next = addr->next;
                EXIT_NOW();
            }
        }
    }

    EXIT_NOW(error = NS_ERROR_NOT_FOUND);

exit:

    if (error != NS_ERROR_NOT_FOUND) {
        if (ip6_netif->addr_callback != NULL) {
            ip6_netif->addr_callback(&addr->addr, IP6_NETIF_MULTICAST_PREFIX_LENGTH, false, ip6_netif->addr_callback_context);
        }
        notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_UNSUBSRCRIBED);
    }

    return error;
}

ns_error_t
ip6_netif_get_next_external_multicast(ip6_netif_t *ip6_netif, uint8_t *iterator, ip6_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    size_t num = NS_ARRAY_LENGTH(ip6_netif->ext_multicast_addrs);
    ip6_netif_multicast_addr_t *entry;

    VERIFY_OR_EXIT(*iterator < num);

    // find an available entry in the `ext_multicast_addrs` array.
    for (uint8_t i = *iterator; i < num; i++) {
        entry = &ip6_netif->ext_multicast_addrs[i];
        // in an unused/available entry `next` points back to the entry itself.
        if (entry->next != entry) {
            *addr = *ip6_netif_multicast_addr_get_addr(entry);
            *iterator = i + 1;
            EXIT_NOW(error = NS_ERROR_NONE);
        }
    }

exit:
    return error;
}

ns_error_t
ip6_netif_subscribe_external_multicast(ip6_netif_t *ip6_netif, const ip6_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_netif_multicast_addr_t *entry;
    size_t num = NS_ARRAY_LENGTH(ip6_netif->ext_multicast_addrs);
    uint8_t i = 0;

    if (ip6_netif_is_multicast_subscribed(ip6_netif, addr)) {
        EXIT_NOW(error = NS_ERROR_ALREADY);
    }

    // find an available entry in the `ext_multicast_addrs` array
    for (entry = &ip6_netif->ext_multicast_addrs[0]; num > 0; num--, entry++, i++) {
        // in an unused/available entry, `next` points back to the entry itself
        if (entry->next == entry) {
            break;
        }
    }

    VERIFY_OR_EXIT(num > 0, error = NS_ERROR_NO_BUFS);

    // copy the address into the available entry and add it to linked list.
    ip6_netif->ext_multicast_addrs[i].addr = *addr;
    ip6_netif->ext_multicast_addrs[i].next = ip6_netif->multicast_addrs;
    ip6_netif->multicast_addrs = &ip6_netif->ext_multicast_addrs[i];

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_SUBSRCRIBED);

exit:
    return error;
}

ns_error_t
ip6_netif_unsubscribe_external_multicast(ip6_netif_t *ip6_netif, const ip6_addr_t *addr)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_netif_multicast_addr_t *entry;
    ip6_netif_multicast_addr_t *last = NULL;
    size_t num = NS_ARRAY_LENGTH(ip6_netif->ext_multicast_addrs);

    for (entry = ip6_netif->multicast_addrs; entry; entry = ip6_netif_multicast_addr_get_next(entry)) {
        if (ip6_addr_is_equal(ip6_netif_multicast_addr_get_addr(entry), addr)) {
            VERIFY_OR_EXIT((entry >= &ip6_netif->ext_multicast_addrs[0]) &&
                           (entry < &ip6_netif->ext_multicast_addrs[num]));
            if (last) {
                last->next = ip6_netif_multicast_addr_get_next(entry);
            } else {
                ip6_netif->multicast_addrs = ip6_netif_multicast_addr_get_next(entry);
            }
            break;
        }
        last = entry;
    }

    VERIFY_OR_EXIT(entry != NULL, error = NS_ERROR_NOT_FOUND);

    // to mark the address entry as unused/available, set the `next` pointer back to the entry itself
    entry->next = entry;

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_UNSUBSRCRIBED);

exit:
    return error;
}

void
ip6_netif_unsubscribe_all_external_multicast(ip6_netif_t *ip6_netif)
{
    size_t num = NS_ARRAY_LENGTH(ip6_netif->ext_multicast_addrs);

    for (ip6_netif_multicast_addr_t *entry = &ip6_netif->ext_multicast_addrs[0]; num > 0; num--, entry++) {
        // in unused entries, the `next` points back to the entry itself
        if (entry->next != entry) {
            ip6_netif_unsubscribe_external_multicast(ip6_netif, ip6_netif_multicast_addr_get_addr(entry));
        }
    }
}

bool
ip6_netif_is_multicast_promiscuous_enabled(ip6_netif_t *ip6_netif)
{
    return ip6_netif->multicast_promiscuous;
}

void
ip6_netif_set_multicast_promiscuous(ip6_netif_t *ip6_netif, bool enabled)
{
    ip6_netif->multicast_promiscuous = enabled;
}

ns_error_t
ip6_netif_send_message(ip6_netif_t *ip6_netif, message_t message)
{
    (void)ip6_netif;
    (void)message;
    return NS_ERROR_NOT_IMPLEMENTED;
}

ns_error_t
ip6_netif_get_link_addr(ip6_netif_t *ip6_netif, ip6_link_addr_t *addr)
{
    (void)ip6_netif;
    (void)addr;
    return NS_ERROR_NOT_IMPLEMENTED;
}

ns_error_t
ip6_netif_route_lookup(ip6_netif_t *ip6_netif,
                       const ip6_addr_t *source,
                       const ip6_addr_t *destination,
                       uint8_t *prefix_match)
{
    (void)ip6_netif;
    (void)source;
    (void)destination;
    (void)prefix_match;
    return NS_ERROR_NOT_IMPLEMENTED;
}

void
ip6_netif_subscribe_all_nodes_multicast(ip6_netif_t *ip6_netif)
{
    assert(ip6_netif->multicast_addrs == NULL);

    ip6_netif->multicast_addrs = (ip6_netif_multicast_addr_t *)((ns_netif_multicast_addr_t *)(&ip6_netif->link_local_all_nodes_multicast_addr));

    if (ip6_netif->addr_callback != NULL) {
        for (const ns_netif_multicast_addr_t *entry = &ip6_netif->link_local_all_nodes_multicast_addr;
             entry != NULL;
             entry = entry->next) {
            ip6_netif->addr_callback(&entry->addr, IP6_NETIF_MULTICAST_PREFIX_LENGTH, true,
                                     ip6_netif->addr_callback_context);
        }
    }

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_SUBSRCRIBED);
}

void
ip6_netif_unsubscribe_all_nodes_multicast(ip6_netif_t *ip6_netif)
{
    assert(ip6_netif->multicast_addrs == NULL ||
           ip6_netif->multicast_addrs == &ip6_netif->link_local_all_nodes_multicast_addr);

    ip6_netif->multicast_addrs = NULL;

    if (ip6_netif->addr_callback != NULL) {
        for (const ns_netif_multicast_addr_t *entry = &ip6_netif->link_local_all_nodes_multicast_addr;
             entry != NULL;
             entry = entry->next) {
            ip6_netif->addr_callback(&entry->addr, IP6_NETIF_MULTICAST_PREFIX_LENGTH, false,
                                     ip6_netif->addr_callback_context);
        }
    }

    notifier_signal(instance_get_notifier(ip6_netif->instance), NS_CHANGED_IP6_MULTICAST_UNSUBSRCRIBED);
}
