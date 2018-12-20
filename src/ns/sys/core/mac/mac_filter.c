#include "ns/sys/core/mac/mac_filter.h"
#include "ns/sys/core/common/code_utils.h"

#if NS_ENABLE_MAC_FILTER

// --- private functions declarations
static mac_filter_entry_t *
mac_filter_find_entry(mac_filter_t *filter, const mac_ext_addr_t *ext_addr);

static mac_filter_entry_t *
mac_filter_find_avail_entry(mac_filter_t *filter);

// --- mac filter functions
void
mac_filter_ctor(mac_filter_t *filter)
{
    filter->addr_mode = NS_MAC_FILTER_ADDRESS_MODE_DISABLED;
    filter->rss_in = NS_MAC_FILTER_FIXED_RSS_DISABLED;

    for (int i = 0; i < mac_filter_get_max_entries(filter); i++) {
        memset(&filter->entries[i], 0, sizeof(mac_filter_entry_t));
        filter->entries[i].filtered = false;
        filter->entries[i].rss_in = NS_MAC_FILTER_FIXED_RSS_DISABLED;
    }
}

uint8_t
mac_filter_get_max_entries(mac_filter_t *filter)
{
    (void)filter;
    return MAC_FILTER_MAX_ENTRIES;
}

ns_mac_filter_addr_mode_t
mac_filter_get_addr_mode(mac_filter_t *filter)
{
    return filter->addr_mode;
}

ns_error_t
mac_filter_set_addr_mode(mac_filter_t *filter, ns_mac_filter_addr_mode_t mode)
{
    ns_error_t error = NS_ERROR_NONE;
    VERIFY_OR_EXIT(mode == NS_MAC_FILTER_ADDRESS_MODE_DISABLED ||
                   mode == NS_MAC_FILTER_ADDRESS_MODE_WHITELIST ||
                   mode == NS_MAC_FILTER_ADDRESS_MODE_BLACKLIST,
                   error = NS_ERROR_INVALID_ARGS);
    filter->addr_mode = mode;
exit:
    return error;
}

ns_error_t
mac_filter_add_addr(mac_filter_t *filter, const mac_ext_addr_t *ext_addr)
{
    ns_error_t error = NS_ERROR_NONE;
    mac_filter_entry_t *entry = mac_filter_find_entry(filter, ext_addr);
    if (entry == NULL) {
        VERIFY_OR_EXIT((entry = mac_filter_find_avail_entry(filter)) != NULL, error = NS_ERROR_NO_BUFS);
        entry->ext_addr = *ext_addr;
    }
    if (entry->filtered) {
        EXIT_NOW(error = NS_ERROR_ALREADY);
    }
    entry->filtered = true;
exit:
    return error;
}

ns_error_t
mac_filter_remove_addr(mac_filter_t *filter, const mac_ext_addr_t *ext_addr)
{
    ns_error_t error = NS_ERROR_NONE;
    mac_filter_entry_t *entry = mac_filter_find_entry(filter, ext_addr);
    if (entry == NULL || !entry->filtered) {
        EXIT_NOW(error = NS_ERROR_NOT_FOUND);
    }
    entry->filtered = false;
exit:
    return error;
}

void
mac_filter_clear_addresses(mac_filter_t *filter)
{
    for (uint8_t i = 0; i < mac_filter_get_max_entries(filter); i++) {
        filter->entries[i].filtered = false;
    }
}

ns_error_t
mac_filter_get_next_addr(mac_filter_t *filter, ns_mac_filter_iterator_t *iterator, mac_filter_entry_t *entry)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    uint8_t i = *(uint8_t *)iterator;
    for (; i < mac_filter_get_max_entries(filter); i++) {
        if (filter->entries[i].filtered) {
            *entry = filter->entries[i];
            *iterator = ++i;
            EXIT_NOW(error = NS_ERROR_NONE);
        }
    }
exit:
    return error;
}

ns_error_t
mac_filter_add_rss_in(mac_filter_t *filter, const mac_ext_addr_t *ext_addr, int8_t rss)
{
    ns_error_t error = NS_ERROR_NONE;
    if (ext_addr == NULL) {
        filter->rss_in = rss;
        EXIT_NOW();
    } else {
        mac_filter_entry_t *entry = mac_filter_find_entry(filter, ext_addr);
        if (entry == NULL) {
            VERIFY_OR_EXIT((entry = mac_filter_find_avail_entry(filter)) != NULL, error = NS_ERROR_NO_BUFS);
            entry->ext_addr = *ext_addr;
        }
        entry->rss_in = rss;
    }
exit:
    return error;
}

ns_error_t
mac_filter_remove_rss_in(mac_filter_t *filter, const mac_ext_addr_t *ext_addr)
{
    ns_error_t error = NS_ERROR_NONE;
    if (ext_addr == NULL) {
        filter->rss_in = NS_MAC_FILTER_FIXED_RSS_DISABLED;
    } else {
        mac_filter_entry_t *entry = mac_filter_find_entry(filter, ext_addr);
        VERIFY_OR_EXIT(entry != NULL, error = NS_ERROR_NOT_FOUND);
        entry->rss_in = NS_MAC_FILTER_FIXED_RSS_DISABLED;
    }
exit:
    return error;
}

void
mac_filter_clear_rss_in(mac_filter_t *filter)
{
    filter->rss_in = NS_MAC_FILTER_FIXED_RSS_DISABLED;
    for (uint8_t i = 0; i < mac_filter_get_max_entries(filter); i++) {
        filter->entries[i].rss_in = NS_MAC_FILTER_FIXED_RSS_DISABLED;
    }
}

ns_error_t
mac_filter_get_next_rss_in(mac_filter_t *filter,
                           ns_mac_filter_iterator_t *iterator,
                           mac_filter_entry_t *entry)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    uint8_t i = *(uint8_t *)iterator;
    for (; i < mac_filter_get_max_entries(filter); i++) {
        if (filter->entries[i].rss_in != NS_MAC_FILTER_FIXED_RSS_DISABLED) {
            *entry = filter->entries[i];
            *iterator = ++i;
            EXIT_NOW(error = NS_ERROR_NONE);
        }
    }
    // return default rss_in setting if no more rss_in filter entry
    if (i == mac_filter_get_max_entries(filter) && filter->rss_in != NS_MAC_FILTER_FIXED_RSS_DISABLED) {
        memset(&entry->ext_addr, 0xff, NS_EXT_ADDRESS_SIZE);
        entry->rss_in = filter->rss_in;
        *iterator = ++i;
        EXIT_NOW(error = NS_ERROR_NONE);
    }
exit:
    return error;
}

ns_error_t
mac_filter_apply(mac_filter_t *filter, const mac_ext_addr_t *ext_addr, int8_t *rss)
{
    ns_error_t error = NS_ERROR_NONE;

    mac_filter_entry_t *entry = mac_filter_find_entry(filter, ext_addr);

    // assign the default rss_in setting for all receiving messages first
    *rss = filter->rss_in;

    // check address filter
    if (filter->addr_mode == NS_MAC_FILTER_ADDRESS_MODE_WHITELIST) {
        VERIFY_OR_EXIT(entry != NULL && entry->filtered, error = NS_ERROR_ADDRESS_FILTERED);
    } else if (filter->addr_mode == NS_MAC_FILTER_ADDRESS_MODE_BLACKLIST) {
        VERIFY_OR_EXIT(entry == NULL || !entry->filtered, error = NS_ERROR_ADDRESS_FILTERED);
    }

    // not override the default rss_in setting if no specific rss_in on the extended address
    if (entry != NULL && entry->rss_in != NS_MAC_FILTER_FIXED_RSS_DISABLED) {
        *rss = entry->rss_in;
    }

exit:
    return error;
}

// --- private functions
static mac_filter_entry_t *
mac_filter_find_entry(mac_filter_t *filter, const mac_ext_addr_t *ext_addr)
{
    mac_filter_entry_t *entry = NULL;
    for (uint8_t i = 0; i < mac_filter_get_max_entries(filter); i++) {
        if ((filter->entries[i].filtered ||
             filter->entries[i].rss_in != NS_MAC_FILTER_FIXED_RSS_DISABLED) &&
            (ext_addr == &filter->entries[i].ext_addr)) {
            EXIT_NOW(entry = &filter->entries[i]);
        }
    }
exit:
    return entry;
}

static mac_filter_entry_t *
mac_filter_find_avail_entry(mac_filter_t *filter)
{
    mac_filter_entry_t *entry = NULL;
    for (uint8_t i = 0; i < mac_filter_get_max_entries(filter); i++) {
        if (!filter->entries[i].filtered &&
            filter->entries[i].rss_in == NS_MAC_FILTER_FIXED_RSS_DISABLED) {
            EXIT_NOW(entry = &filter->entries[i]);
        }
    }
exit:
    return entry;
}

#endif // NS_ENABLE_MAC_FILTER
