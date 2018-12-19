#ifndef NS_CORE_MAC_MAC_FILTER_H_
#define NS_CORE_MAC_MAC_FILTER_H_

#include "ns/sys/core/core-config.h"
#include "ns/sys/core/mac/mac_frame.h"
#include <string.h>

#if NS_ENABLE_MAC_FILTER

typedef ns_mac_filter_entry_t mac_filter_entry_t;

enum {
    MAC_FILTER_MAX_ENTRIES = NS_CONFIG_MAC_FILTER_SIZE,
};

typedef struct _mac_filter {
    mac_filter_entry_t entries[MAC_FILTER_MAX_ENTRIES];
    ns_mac_filter_addr_mode_t addr_mode;
    int8_t rss_in;
} mac_filter_t;

void
mac_filter_ctor(mac_filter_t *mac_filter);

uint8_t
mac_filter_get_max_entries(mac_filter_t *mac_filter);

ns_mac_filter_addr_mode_t
mac_filter_get_addr_mode(mac_filter_t *mac_filter);

ns_error_t
mac_filter_set_addr_mode(mac_filter_t *mac_filter, ns_mac_filter_addr_mode_t mode);

ns_error_t
mac_filter_add_addr(mac_filter_t *mac_filter, const ext_addr_t *ext_addr);

ns_error_t
mac_filter_remove_addr(mac_filter_t *mac_filter, const ext_addr_t *ext_addr);

void
mac_filter_clear_addresses(mac_filter_t *mac_filter);

ns_error_t
mac_filter_get_next_addr(mac_filter_t *mac_filter, ns_mac_filter_iterator_t *iterator, mac_filter_entry_t *entry);

ns_error_t
mac_filter_add_rss_in(mac_filter_t *mac_filter, const ext_addr_t *ext_addr, int8_t rss);

ns_error_t
mac_filter_remove_rss_in(mac_filter_t *mac_filter, const ext_addr_t *ext_addr);

void
mac_filter_clear_rss_in(mac_filter_t *mac_filter);

ns_error_t
mac_filter_get_next_rss_in(mac_filter_t *mac_filter,
                           ns_mac_filter_iterator_t *iterator,
                           mac_filter_entry_t *entry);

ns_error_t
mac_filter_apply(mac_filter_t *mac_filter, const ext_addr_t *ext_addr, int8_t *rss);

#endif // NS_ENABLE_MAC_FILTER

#endif // NS_CORE_MAC_MAC_FILTER_H_
