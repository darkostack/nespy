#include "ns/sys/core/thread/link_quality.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/code_utils.h"
#include <string.h>

STRING(rss_averager_info, RSS_AVERAGER_STRING_SIZE);
STRING(link_quality_info, LINK_QUALITY_INFO_STRING_SIZE);

// This array gives the decimal point digits representing 0/8, 1/8, ..., 7/8 (does not include the '.').
static const char *const s_digits_string[8] = {
    // 0/8,  1/8,   2/8,   3/8,   4/8,   5/8,   6/8,   7/8
    "0", "125", "25", "375", "5", "625", "75", "875"};

// --- private functionds declarations
static void
set_link_quality(link_quality_info_t *info, uint8_t link_quality);

static uint8_t
calculate_link_quality(link_quality_info_t *info, uint8_t link_margin, uint8_t last_link_quality);

// --- success rate tracker functions
void
success_rate_tracker_reset(success_rate_tracker_t *tracker)
{
    tracker->failure_rate = 0;
}

void
success_rate_tracker_add_sample(success_rate_tracker_t *tracker, bool success, uint16_t weight)
{
    uint32_t old_average = tracker->failure_rate;
    uint32_t new_value = (success) ? 0 : SUCCESS_RATE_TRACKER_MAX_RATE_VALUE;
    uint32_t n = weight;

    // `n/2` is added to the sum to ensure rounding the value to the nearest integer when dividing by `n`
    // (e.g., 1.2 -> 1, 3.5 -> 4).

    tracker->failure_rate = (uint16_t)(((old_average * (n - 1)) + new_value + (n / 2)) / n);
}

uint16_t
success_rate_tracker_get_failure_rate(success_rate_tracker_t *tracker)
{
    return tracker->failure_rate;
}

uint16_t
success_rate_tracker_get_success_rate(success_rate_tracker_t *tracker)
{
    return SUCCESS_RATE_TRACKER_MAX_RATE_VALUE - tracker->failure_rate;
}

// --- rss averager functions
void
rss_averager_reset(rss_averager_t *averager)
{
    averager->average = 0;
    averager->count = 0;
}

bool
rss_averager_has_average(rss_averager_t *averager)
{
    return (averager->count != 0);
}

ns_error_t
rss_averager_add(rss_averager_t *averager, int8_t rss)
{
    ns_error_t error = NS_ERROR_NONE;
    uint16_t new_value;
    uint16_t old_average;

    VERIFY_OR_EXIT(rss != NS_RADIO_RSSI_INVALID, error = NS_ERROR_INVALID_ARGS);

    // restrict the RSS value to the closed range [0, -128],
    // so the RSS times precision multiple can fit in 11 bits.
    if (rss > 0) {
        rss = 0;
    }

    // multiply the RSS value by a precision multiple (currently -8).
    new_value = (uint16_t)(-rss);
    new_value <<= RSS_AVERAGER_PRECISION_BIT_SHIFT;

    old_average = averager->average;

    if (averager->count == 0) {
        averager->count++;
        averager->average = new_value;
    } else if (averager->count < (1 << RSS_AVERAGER_COEFF_BIT_SHIFT) - 1) {
        averager->count++;
        // Maintain arithmetic mean.
        // newAverage = newValue * (1/mCount) + oldAverage * ((mCount -1)/mCount)
        averager->average = (uint16_t)(((old_average * (averager->count - 1)) + new_value) / averager->count);
    } else {
        // Maintain exponentially weighted moving average using coefficient of (1/2^kCoeffBitShift).
        // newAverage = + newValue * 1/2^j + oldAverage * (1 - 1/2^j), for j = kCoeffBitShift.
        averager->average = (uint16_t)(((old_average << RSS_AVERAGER_COEFF_BIT_SHIFT) - old_average + new_value) >> RSS_AVERAGER_COEFF_BIT_SHIFT);
    }

exit:
    return error;
}

int8_t
rss_averager_get_average(rss_averager_t *averager)
{
    int8_t average;

    VERIFY_OR_EXIT(averager->count != 0, average = NS_RADIO_RSSI_INVALID);

    average = -(int8_t)(averager->average >> RSS_AVERAGER_PRECISION_BIT_SHIFT);

    // Check for possible round up (e.g., average of -71.5 --> -72)

    if ((averager->average & RSS_AVERAGER_PRECISION_BIT_MASK) >= (RSS_AVERAGER_PRECISION >> 1)) {
        average--;
    }

exit:
    return average;
}

uint16_t
rss_averager_get_raw(rss_averager_t *averager)
{
    return averager->average;
}

string_t *
rss_averager_to_string(rss_averager_t *averager)
{
    string_t *rss_averager_string = &rss_averager_info_string;
    string_clear(rss_averager_string);
    VERIFY_OR_EXIT(averager->count != 0);
    string_set(rss_averager_string,"%d.%s",
               -(averager->average >> RSS_AVERAGER_PRECISION_BIT_SHIFT), 
               s_digits_string[averager->average & RSS_AVERAGER_PRECISION_BIT_MASK]);
exit:
    return rss_averager_string;
}

// --- link quality info functions
void
link_quality_info_clear(link_quality_info_t *info)
{
    rss_averager_reset(&info->rss_averager);
    set_link_quality(info, 0);
    info->last_rss = NS_RADIO_RSSI_INVALID;
#if NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING
    success_rate_tracker_reset(&info->frame_error_rate);
    success_rate_tracker_reset(&info->message_error_rate);
#endif
}

void
link_quality_info_add_rss(link_quality_info_t *info, int8_t noise_floor, int8_t rss)
{
    uint8_t old_link_quality = LINK_QUALITY_NO_LINK_QUALITY;

    VERIFY_OR_EXIT(rss != NS_RADIO_RSSI_INVALID);

    info->last_rss = rss;

    if (rss_averager_has_average(&info->rss_averager)) {
        old_link_quality = link_quality_info_get_link_quality(info);
    }

    SUCCESS_OR_EXIT(rss_averager_add(&info->rss_averager, rss));

    set_link_quality(info, calculate_link_quality(info, link_quality_info_get_link_margin(info, noise_floor), old_link_quality));

exit:
    return;
}

int8_t
link_quality_info_get_average_rss(link_quality_info_t *info)
{
    return rss_averager_get_average(&info->rss_averager);
}

uint16_t
link_quality_info_get_average_rss_raw(link_quality_info_t *info)
{
    return rss_averager_get_raw(&info->rss_averager);
}

string_t *
link_quality_info_to_string(link_quality_info_t *info)
{
    string_t *link_quality_string = &link_quality_info_string;
    string_clear(link_quality_string);
    string_set(link_quality_string, "ave-rss:%s, last-rss:%d, link-quality:%d",
               string_as_c_string(rss_averager_to_string(&info->rss_averager)),
               link_quality_info_get_last_rss(info),
               link_quality_info_get_link_quality(info));
    return link_quality_string;
}

uint8_t
link_quality_info_get_link_margin(link_quality_info_t *info, int8_t noise_floor)
{
    return link_quality_info_convert_rss_to_link_margin(info, noise_floor, link_quality_info_get_average_rss(info));
}

uint8_t
link_quality_info_get_link_quality(link_quality_info_t *info)
{
    return info->link_quality;
}

int8_t
link_quality_info_get_last_rss(link_quality_info_t *info)
{
    return info->last_rss;
}

#if NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING
void
link_quality_info_add_frame_tx_status(link_quality_info_t *info, bool tx_status)
{
    success_rate_tracker_add_sample(&info->frame_error_rate, tx_status, NS_CONFIG_FRAME_TX_ERR_RATE_AVERAGING_WINDOW);
}

void
link_quality_info_add_message_tx_status(link_quality_info_t *info, bool tx_status)
{
    success_rate_tracker_add_sample(&info->message_error_rate, tx_status, NS_CONFIG_IPV6_TX_ERR_RATE_AVERAGING_WINDOW);
}

uint16_t
link_quality_info_get_frame_error_rate(link_quality_info_t *info)
{
    return success_rate_tracker_get_failure_rate(&info->frame_error_rate);
}

uint16_t
link_quality_info_get_message_error_rate(link_quality_info_t *info)
{
    return success_rate_tracker_get_failure_rate(&info->message_error_rate);
}
#endif // NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING

uint8_t
link_quality_info_convert_rss_to_link_margin(link_quality_info_t *info, int8_t noise_floor, int8_t rss)
{
    (void)info;
    int8_t link_margin = rss - noise_floor;
    if (link_margin < 0 || rss == NS_RADIO_RSSI_INVALID) {
        link_margin = 0;
    }
    return (uint8_t)link_margin;
}

uint8_t
link_quality_info_convert_link_margin_to_link_quality(link_quality_info_t *info, uint8_t link_margin)
{
    return calculate_link_quality(info, link_margin, LINK_QUALITY_NO_LINK_QUALITY);
}

uint8_t
link_quality_info_convert_rss_to_link_quality(link_quality_info_t *info, int8_t noise_floor, int8_t rss)
{
    return link_quality_info_convert_link_margin_to_link_quality(info, link_quality_info_convert_rss_to_link_margin(info, noise_floor, rss));
}

int8_t
link_quality_info_convert_link_quality_to_rss(link_quality_info_t *info, int8_t noise_floor, uint8_t link_quality)
{
    uint8_t link_margin = 0;
    switch (link_quality) {
    case 3:
        link_margin = LINK_QUALITY_3_LINK_MARGIN;
        break;
    case 2:
        link_margin = LINK_QUALITY_2_LINK_MARGIN;
        break;
    case 1:
        link_margin = LINK_QUALITY_1_LINK_MARGIN;
        break;
    default:
        link_margin = LINK_QUALITY_0_LINK_MARGIN;
        break;
    }

    return link_margin + noise_floor;
}

// --- private functions
static void
set_link_quality(link_quality_info_t *info, uint8_t link_quality)
{
    info->link_quality = link_quality;
}

static uint8_t
calculate_link_quality(link_quality_info_t *info, uint8_t link_margin, uint8_t last_link_quality)
{
    uint8_t threshold1, threshold2, threshold3;
    uint8_t link_quality = 0;

    threshold1 = LINK_QUALITY_THRESHOLD_1;
    threshold2 = LINK_QUALITY_THRESHOLD_2;
    threshold3 = LINK_QUALITY_THRESHOLD_3;

    // Apply the hysteresis threshold based on the last link quality value.

    switch (last_link_quality) {
    case 0:
        threshold1 += LINK_QUALITY_HYSTERESIS_THRESHOLD;
        // fall-through
    case 1:
        threshold2 += LINK_QUALITY_HYSTERESIS_THRESHOLD;
        // fall-through
    case 2:
        threshold3 += LINK_QUALITY_HYSTERESIS_THRESHOLD;
        // fall-through
    default:
        break;
    }

    if (link_margin > threshold3) {
        link_quality = 3;
    } else if (link_margin > threshold2) {
        link_quality = 2;
    } else if (link_margin > threshold1) {
        link_quality = 1;
    }

    return link_quality;
}
