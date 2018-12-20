#ifndef NS_CORE_THREAD_LINK_QUALITY_H_
#define NS_CORE_THREAD_LINK_QUALITY_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/platform/radio.h"
#include "ns/sys/core/common/string.h"

typedef struct _success_rate_tracker success_rate_tracker_t;
typedef struct _rss_averager rss_averager_t;
typedef struct _link_quality_info link_quality_info_t;

enum {
    SUCCESS_RATE_TRACKER_MAX_RATE_VALUE = 0xffff,
};

enum {
    SUCCESS_RATE_TRACKER_DEFAULT_WEIGHT = 64,
};

struct _success_rate_tracker {
    uint16_t failure_rate;
};

enum {
    RSS_AVERAGER_STRING_SIZE = 10,
};

enum {
    RSS_AVERAGER_PRECISION_BIT_SHIFT = 3,
    RSS_AVERAGER_PRECISION           = (1 << RSS_AVERAGER_PRECISION_BIT_SHIFT),
    RSS_AVERAGER_PRECISION_BIT_MASK  = (RSS_AVERAGER_PRECISION - 1),

    RSS_AVERAGER_COEFF_BIT_SHIFT = 3,
};

struct _rss_averager {
    uint16_t average : 11;
    uint16_t count : 5;
};

enum {
    LINK_QUALITY_INFO_STRING_SIZE = 50,
};

enum {
    LINK_QUALITY_THRESHOLD_3          = 20,
    LINK_QUALITY_THRESHOLD_2          = 10,
    LINK_QUALITY_THRESHOLD_1          = 2,
    LINK_QUALITY_HYSTERESIS_THRESHOLD = 2,

    // constants for test:
    LINK_QUALITY_3_LINK_MARGIN = 50,
    LINK_QUALITY_2_LINK_MARGIN = 15,
    LINK_QUALITY_1_LINK_MARGIN = 5,
    LINK_QUALITY_0_LINK_MARGIN = 0,

    LINK_QUALITY_NO_LINK_QUALITY = 0xff,
};

struct _link_quality_info {
    rss_averager_t rss_averager;
    uint8_t link_quality;
    int8_t last_rss;
#if NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING
    success_rate_tracker_t frame_error_rate;
    success_rate_tracker_t message_error_rate;
#endif
};

void
success_rate_tracker_reset(success_rate_tracker_t *tracker);

void
success_rate_tracker_add_sample(success_rate_tracker_t *tracker, bool success, uint16_t weight);

uint16_t
success_rate_tracker_get_failure_rate(success_rate_tracker_t *tracker);

uint16_t
success_rate_tracker_get_success_rate(success_rate_tracker_t *tracker);

void
rss_averager_reset(rss_averager_t *averager);

bool
rss_averager_has_average(rss_averager_t *averager);

ns_error_t
rss_averager_add(rss_averager_t *averager, int8_t rss);

int8_t
rss_averager_get_average(rss_averager_t *averager);

uint16_t
rss_averager_get_raw(rss_averager_t *averager);

string_t *
rss_averager_to_string(rss_averager_t *averager);

void
link_quality_info_clear(link_quality_info_t *info);

void
link_quality_info_add_rss(link_quality_info_t *info, int8_t noise_floor, int8_t rss);

int8_t
link_quality_info_get_average_rss(link_quality_info_t *info);

uint16_t
link_quality_info_get_average_rss_raw(link_quality_info_t *info);

string_t *
link_quality_info_to_string(link_quality_info_t *info);

uint8_t
link_quality_info_get_link_margin(link_quality_info_t *info, int8_t noise_floor);

uint8_t
link_quality_info_get_link_quality(link_quality_info_t *info);

int8_t
link_quality_info_get_last_rss(link_quality_info_t *info);

#if NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING
void
link_quality_info_add_frame_tx_status(link_quality_info_t *info, bool tx_status);

void
link_quality_info_add_message_tx_status(link_quality_info_t *info, bool tx_status);

uint16_t
link_quality_info_get_frame_error_rate(link_quality_info_t *info);

uint16_t
link_quality_info_get_message_error_rate(link_quality_info_t *info);
#endif // NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING

uint8_t
link_quality_info_convert_rss_to_link_margin(link_quality_info_t *info, int8_t noise_floor, int8_t rss);

uint8_t
link_quality_info_convert_link_margin_to_link_quality(link_quality_info_t *info, uint8_t link_margin);

uint8_t
link_quality_info_convert_rss_to_link_quality(link_quality_info_t *info, int8_t noise_floor, int8_t rss);

int8_t
link_quality_info_convert_link_quality_to_rss(link_quality_info_t *info, int8_t noise_floor, uint8_t link_quality);

#endif // NS_CORE_THREAD_LINK_QUALITY_H_
