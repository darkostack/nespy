#include "ns/include/error.h"
#include "ns/sys/core/common/code_utils.h"
#include "ns/sys/core/mac/channel_mask.h"

STRING(mac_channel_mask_info, MAC_CHANNEL_MASK_INFO_STRING_SIZE);

void
mac_channel_mask_ctor(mac_channel_mask_t *channel_mask, uint32_t mask)
{
    channel_mask->mask = mask;
}

void
mac_channel_mask_clear(mac_channel_mask_t *channel_mask)
{
    channel_mask->mask = 0;
}

uint32_t
mac_channel_mask_get_mask(mac_channel_mask_t *channel_mask)
{
    return channel_mask->mask;
}

void
mac_channel_mask_set_mask(mac_channel_mask_t *channel_mask, uint32_t mask)
{
    channel_mask->mask = mask;
}

bool
mac_channel_mask_is_empty(mac_channel_mask_t *channel_mask)
{
    return (channel_mask->mask == 0);
}

bool
mac_channel_mask_is_single_channel(mac_channel_mask_t *channel_mask)
{
    return ((channel_mask->mask != 0) && ((channel_mask->mask & (channel_mask->mask - 1)) == 0));
}

bool
mac_channel_mask_contains_channel(mac_channel_mask_t *channel_mask, uint8_t channel)
{
    return (((1U << channel) & channel_mask->mask) != 0);
}

void
mac_channel_mask_add_channel(mac_channel_mask_t *channel_mask, uint8_t channel)
{
    channel_mask->mask |= (1U << channel);
}

void
mac_channel_mask_remove_channel(mac_channel_mask_t *channel_mask, uint8_t channel)
{
    channel_mask->mask &= ~(1U << channel);
}

void
mac_channel_mask_intersect(mac_channel_mask_t *channel_mask, const mac_channel_mask_t *other_channel_mask)
{
    channel_mask->mask &= other_channel_mask->mask;
}

uint8_t
mac_channel_mask_get_number_of_channels(mac_channel_mask_t *channel_mask)
{
    uint8_t num = 0;
    uint8_t channel = MAC_CHANNEL_MASK_CHANNEL_ITERATOR_FIRST;
    while (mac_channel_mask_get_next_channel(channel_mask, &channel) == NS_ERROR_NONE) {
        num++;
    }
    return num;
}

ns_error_t
mac_channel_mask_get_next_channel(mac_channel_mask_t *channel_mask, uint8_t *channel)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    uint8_t ch = *channel;
    if (ch == MAC_CHANNEL_MASK_CHANNEL_ITERATOR_FIRST) {
        ch = (NS_RADIO_CHANNEL_MIN - 1);
    }
    for (ch++; ch <= NS_RADIO_CHANNEL_MAX; ch++) {
        if (mac_channel_mask_contains_channel(channel_mask, ch)) {
            EXIT_NOW(error = NS_ERROR_NONE);
        }
    }
exit:
    *channel = ch;
    return error;
}

bool
mac_channel_mask_is_equal(mac_channel_mask_t *channel_mask, mac_channel_mask_t *other_channel_mask)
{
    return (channel_mask->mask == other_channel_mask->mask);
}

string_t *
mac_channel_mask_to_string(mac_channel_mask_t *channel_mask)
{
    string_t *channel_mask_string = &mac_channel_mask_info_string;
    uint8_t channel = MAC_CHANNEL_MASK_CHANNEL_ITERATOR_FIRST;
    bool add_comma = false;
    ns_error_t error;

    string_clear(channel_mask_string);
    string_append(channel_mask_string, "{");

    error = mac_channel_mask_get_next_channel(channel_mask, &channel);

    while (error == NS_ERROR_NONE) {
        uint8_t range_start = channel;
        uint8_t range_end = channel;

        while ((error = mac_channel_mask_get_next_channel(channel_mask, &channel)) == NS_ERROR_NONE) {
            if (channel != range_end + 1) {
                break;
            }
            range_end = channel;
        }

        string_append(channel_mask_string, "%s%d", add_comma ? ", " : " ", range_start);
        add_comma = true;

        if (range_start < range_end) {
            string_append(channel_mask_string, "%s%d", range_end == range_start + 1 ? ", " : "-", range_end);
        }
    }

    string_append(channel_mask_string, " }");

    return channel_mask_string;
}
