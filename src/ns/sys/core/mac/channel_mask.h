#ifndef NS_CORE_MAC_CHANNEL_MASK_H_
#define NS_CORE_MAC_CHANNEL_MASK_H_

#include "ns/sys/core/core-config.h"
#include "ns/sys/core/common/string.h"
#include "ns/include/platform/radio.h"
#include <string.h>

#if (NS_RADIO_CHANNEL_MIN >= 32) || (NS_RADIO_CHANNEL_MAX >= 32)
#error `NS_RADIO_CHANNEL_MAX` or `NS_RADIO_CHANNEL_MIN` are larger than 32. `mac_channel_mask_t` use 32 bit mask.
#endif

typedef struct _mac_channel_mask mac_channel_mask_t;

enum {
    MAC_CHANNEL_MASK_CHANNEL_ITERATOR_FIRST = 0xff,
    MAC_CHANNEL_MASK_INFO_STRING_SIZE = 46,
};

struct _mac_channel_mask {
    uint32_t mask;
};

void
mac_channel_mask_ctor(mac_channel_mask_t *channel_mask, uint32_t mask);

void
mac_channel_mask_clear(mac_channel_mask_t *channel_mask);

uint32_t
mac_channel_mask_get_mask(mac_channel_mask_t *channel_mask);

void
mac_channel_mask_set_mask(mac_channel_mask_t *channel_mask, uint32_t mask);

bool
mac_channel_mask_is_empty(mac_channel_mask_t *channel_mask);

bool
mac_channel_mask_is_single_channel(mac_channel_mask_t *channel_mask);

bool
mac_channel_mask_contains_channel(mac_channel_mask_t *channel_mask, uint8_t channel);

void
mac_channel_mask_add_channel(mac_channel_mask_t *channel_mask, uint8_t channel);

void
mac_channel_mask_remove_channel(mac_channel_mask_t *channel_mask, uint8_t channel);

void
mac_channel_mask_intersect(mac_channel_mask_t *channel_mask, const mac_channel_mask_t *other_channel_mask);

uint8_t
mac_channel_mask_get_number_of_channels(mac_channel_mask_t *channel_mask);

ns_error_t
mac_channel_mask_get_next_channel(mac_channel_mask_t *channel_mask, uint8_t *channel);

bool
mac_channel_mask_is_equal(mac_channel_mask_t *channel_mask, mac_channel_mask_t *other_channel_mask);

string_t *
mac_channel_mask_to_string(mac_channel_mask_t *channel_mask);

#endif // NS_CORE_MAC_CHANNEL_MASK_H_
