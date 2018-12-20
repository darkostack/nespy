#include "ns/include/platform/random.h"
#include "ns/sys/core/common/code_utils.h"
#include "ns/sys/core/thread/link_quality.h"
#include "ns/test/unit/test_util.h"
#include <string.h>

#define PRINT_DEBUG 0

#if PRINT_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

enum {
    LINK_QUALITY_TEST_MAX_RSS_VALUE = 0,
    LINK_QUALITY_TEST_MIN_RSS_VALUE = -128,

    LINK_QUALITY_TEST_STRING_BUFFER_SIZE = 80,

    LINK_QUALITY_TEST_RSS_AVERAGE_MAX_DIFF = 16,
    LINK_QUALITY_TEST_NUM_RSS_ADDS = 300,

    LINK_QUALITY_TEST_RAW_AVERAGE_BIT_SHIFT = 3,
    LINK_QUALITY_TEST_RAW_AVERAGE_MULTIPLE  = (1 << LINK_QUALITY_TEST_RAW_AVERAGE_BIT_SHIFT),
    LINK_QUALITY_TEST_RAW_AVERAGE_BIT_MASK  = (1 << LINK_QUALITY_TEST_RAW_AVERAGE_BIT_SHIFT) - 1,
};

#define MIN_RSS(rss1, rss2) (((rss1) < (rss2)) ? (rss1) : (rss2))
#define MAX_RSS(rss1, rss2) (((rss1) < (rss2)) ? (rss2) : (rss1))
#define ABS(value) (((value) >= 0) ? (value) : -(value))

struct _rss_test_data {
    const int8_t *rss_list;
    size_t rss_list_size;
    uint8_t expected_link_quality;
};

static int8_t s_noise_floor = -100; // dBm

// check and verify the raw average RSS value to match the value from get_average()
static ns_error_t
verify_raw_rss_value(int8_t average, uint16_t raw_value)
{
    ns_error_t error = NS_ERROR_NONE;
    if (average != NS_RADIO_RSSI_INVALID) {
        TEST_VERIFY_OR_EXIT(average == -(int16_t)((raw_value + (LINK_QUALITY_TEST_RAW_AVERAGE_MULTIPLE / 2)) >> LINK_QUALITY_TEST_RAW_AVERAGE_BIT_SHIFT),
                            "test link quality info failed - raw value does not match the average.\r\n");
    } else {
        TEST_VERIFY_OR_EXIT(raw_value == 0,
                            "test link quality info failed - raw value does not match the average.\r\n");
    }

exit:
    return error;
}

static void
print_link_quality_info(link_quality_info_t *info)
{
    PRINTF("%s -> PASS\r\n", string_as_c_string(link_quality_info_to_string(info)));
}

static ns_error_t
test_link_quality_data(const struct _rss_test_data *rss_data)
{
    ns_error_t error = NS_ERROR_NONE;
    link_quality_info_t link_info;
    int8_t rss, ave, min, max;
    size_t i;

    PRINTF("- - - - - - - - - - - - - - - - - -\n");

    link_quality_info_clear(&link_info);
    min = LINK_QUALITY_TEST_MIN_RSS_VALUE;
    max = LINK_QUALITY_TEST_MAX_RSS_VALUE;

    for (i = 0; i < rss_data->rss_list_size; i++) {
        rss = rss_data->rss_list[i];
        min = MIN_RSS(rss, min);
        max = MAX_RSS(rss, max);
        link_quality_info_add_rss(&link_info, s_noise_floor, rss);
        TEST_VERIFY_OR_EXIT(link_quality_info_get_last_rss(&link_info) == rss,
                            "test link quality info failed - get_last_rss() is incorrect.\r\n");
        ave = link_quality_info_get_average_rss(&link_info);
        TEST_VERIFY_OR_EXIT(ave >= min,
                       "test link quality info failed - get_average_rss() is smaller than min value.\r\n");
        TEST_VERIFY_OR_EXIT(ave <= max,
                       "test link quality info failed - get_average_rss() is bigger than max value.\r\n");
        TEST_VERIFY_OR_EXIT(verify_raw_rss_value(link_quality_info_get_average_rss(&link_info),
                                link_quality_info_get_average_rss_raw(&link_info)) == NS_ERROR_NONE,
                            "test link quality info failed - get raw rss value.\r\n");
        PRINTF("%02u) add-rss(%4d): ", (unsigned int)i, rss);
        print_link_quality_info(&link_info);
    }

    TEST_VERIFY_OR_EXIT(link_quality_info_get_link_quality(&link_info) == rss_data->expected_link_quality,
                        "test link quality info failed - get_link_quality() is incorrect.\r\n");
exit:
    return error;
}

// check and verify the raw average rss value to match the value from the get_average()
static ns_error_t
verify_raw_rss_averager(rss_averager_t *averager)
{
    ns_error_t error = NS_ERROR_NONE;
    int8_t average = rss_averager_get_average(averager);
    uint16_t raw_value = rss_averager_get_raw(averager);
    if (average != NS_RADIO_RSSI_INVALID) {
        TEST_VERIFY_OR_EXIT(average == -(int16_t)((raw_value + (LINK_QUALITY_TEST_RAW_AVERAGE_MULTIPLE / 2)) >> LINK_QUALITY_TEST_RAW_AVERAGE_BIT_SHIFT),
                            "test link quality info failed - raw value does not match the average.\r\n");
    } else {
        TEST_VERIFY_OR_EXIT(raw_value == 0,
                            "test link quality info failed - raw value does not match the average.\r\n");
   }
exit:
    return error;
}

static void
print_rss_averager(rss_averager_t *averager)
{
    PRINTF("%s -> PASS\r\n", string_as_c_string(rss_averager_to_string(averager)));
}

static int8_t
get_random_rss(void)
{
    uint32_t value;
    value = ns_plat_random_get() % 128;
    return (int8_t)(-value);
}

ns_error_t
test_rss_averager(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("-------------------------- TEST RSS AVERAGER\r\n");

    rss_averager_t rss_averager;
    int8_t rss, rss2, ave;
    int16_t diff;
    size_t i, j, k;
    int16_t sum;
    const int8_t rss_values[] = {
        LINK_QUALITY_TEST_MIN_RSS_VALUE,
        -70,
        -40,
        -41,
        -10,
        LINK_QUALITY_TEST_MAX_RSS_VALUE
    };

    // --- value after initialization/reset
    rss_averager_reset(&rss_averager);

    PRINTF("after reset: ");
    TEST_VERIFY_OR_EXIT(rss_averager_get_average(&rss_averager) == NS_RADIO_RSSI_INVALID,
                        "test link quality info failed - initial value from get_average() is incorrect.\r\n");
    TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                        "test link quality info failed - raw rss averager is incorrect.\r\n");
    print_rss_averager(&rss_averager);

    // --- adding a single value
    rss = -70;
    PRINTF("add-rss(%d): ", rss);
    rss_averager_add(&rss_averager, rss);
    TEST_VERIFY_OR_EXIT(rss_averager_get_average(&rss_averager) == rss,
                        "test link quality info failed - get_average() is incorrect.\r\n");
    TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                        "test link quality info failed - raw rss averager is incorrect.\r\n");
    print_rss_averager(&rss_averager);

    // --- reset
    PRINTF("reset(): ");
    rss_averager_reset(&rss_averager);
    TEST_VERIFY_OR_EXIT(rss_averager_get_average(&rss_averager) == NS_RADIO_RSSI_INVALID,
                        "test link quality info failed - get_average() is incorrect.\r\n");
    TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                        "test link quality info failed - raw rss averager is incorrect.\r\n");
    print_rss_averager(&rss_averager);

    // --- adding the same value many times
    PRINTF("- - - - - - - - - - - - - - - - - -\r\n");

    for (j = 0; j < sizeof(rss_values); j++) {
        rss_averager_reset(&rss_averager);
        rss = rss_values[j];
        PRINTF("add-rss(%4d) %d times: ", rss, LINK_QUALITY_TEST_NUM_RSS_ADDS);
        for (i = 0; i < LINK_QUALITY_TEST_NUM_RSS_ADDS; i++) {
            rss_averager_add(&rss_averager, rss);
            TEST_VERIFY_OR_EXIT(rss_averager_get_average(&rss_averager) == rss,
                                "test link quality info failed - get_average() is incorrect.\r\n");
            TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                                "test link quality info failed - raw rss averager is incorrect.\r\n");
        }
        print_rss_averager(&rss_averager);
    }

    // --- adding two rss values
    PRINTF("- - - - - - - - - - - - - - - - - -\r\n");

    for (j = 0; j < sizeof(rss_values); j++) {
        rss = rss_values[j];
        for (k = 0; k < sizeof(rss_values); k++) {
            if (k == j) {
                continue;
            }
            rss2 = rss_values[k];
            rss_averager_reset(&rss_averager);
            rss_averager_add(&rss_averager, rss);
            rss_averager_add(&rss_averager, rss2);
            PRINTF("add-rss(%4d), add-rss(%d): ", rss, rss2);
            TEST_VERIFY_OR_EXIT(rss_averager_get_average(&rss_averager) == ((rss + rss2) >> 1),
                                "test link quality info failed - get_average() is incorrect.\r\n");
            TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                                "test link quality info failed - raw rss averager is incorrect.\r\n");
            print_rss_averager(&rss_averager);
        }
    }

    // --- adding one value many times and different value once
    PRINTF("- - - - - - - - - - - - - - - - - -\r\n");

    for (j = 0; j < sizeof(rss_values); j++) {
        rss = rss_values[j];
        for (k = 0; k < sizeof(rss_values); k++) {
            if (k == j) {
                continue;
            }
            rss2 = rss_values[k];
            rss_averager_reset(&rss_averager);
            for (i = 0; i < LINK_QUALITY_TEST_NUM_RSS_ADDS; i++) {
                rss_averager_add(&rss_averager, rss);
            }
            rss_averager_add(&rss_averager, rss2);
            PRINTF("add-rss(%4d) %d times, add-rss(%4d): ", rss, LINK_QUALITY_TEST_NUM_RSS_ADDS, rss2);
            ave = rss_averager_get_average(&rss_averager);
            TEST_VERIFY_OR_EXIT(ave >= MIN_RSS(rss, rss2),
                                "test link quality info failed - get_average() smaller than min value.\r\n");
            TEST_VERIFY_OR_EXIT(ave <= MAX_RSS(rss, rss2),
                                "test link quality info failed - get_average() bigger than max value.\r\n");
            TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                                "test link quality info failed - raw rss averager is incorrect.\r\n");
            print_rss_averager(&rss_averager);
        }
    }

    // --- adding two alternating values many times
    PRINTF("- - - - - - - - - - - - - - - - - -\r\n");

    for (j = 0; j < sizeof(rss_values); j++) {
        rss = rss_values[j];
        for (k = 0; k < sizeof(rss_values); k++) {
            if (k == j) {
                continue;
            }
            rss2 = rss_values[k];
            rss_averager_reset(&rss_averager);
            for (i = 0; i < LINK_QUALITY_TEST_NUM_RSS_ADDS; i++) {
                rss_averager_add(&rss_averager, rss);
                rss_averager_add(&rss_averager, rss2);
                ave = rss_averager_get_average(&rss_averager);
                TEST_VERIFY_OR_EXIT(ave >= MIN_RSS(rss, rss2),
                                    "test link quality info failed - get_average() smaller than min value.\r\n");
                TEST_VERIFY_OR_EXIT(ave <= MAX_RSS(rss, rss2),
                                    "test link quality info failed - get_average() bigger than max value.\r\n");
                diff = ave;
                diff -= (rss + rss2) >> 1;
                TEST_VERIFY_OR_EXIT(ABS(diff) <= LINK_QUALITY_TEST_RSS_AVERAGE_MAX_DIFF,
                                    "test link quality info failed - get_average() is incorrect().\r\n");
                TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                                    "test link quality info failed - raw rss averager is incorrect.\r\n");
            }
            PRINTF("[add-rss(%4d), add-rss(%4d)] %d times: ", rss, rss2, LINK_QUALITY_TEST_NUM_RSS_ADDS);
            print_rss_averager(&rss_averager);
        }
    }

    // for the first 8 values the average should be the arithmetic mean
    PRINTF("- - - - - - - - - - - - - - - - - -\r\n");

    for (i = 0; i < 1000; i++) {
        double mean;
        rss_averager_reset(&rss_averager);
        sum = 0;
        PRINTF("\r\n");
        for (j = 1; j <= 8; j++) {
            rss = get_random_rss();
            rss_averager_add(&rss_averager, rss);
            sum += rss;
            mean = (double)(sum) / j;
            TEST_VERIFY_OR_EXIT(ABS(rss_averager_get_average(&rss_averager) - mean) < 1,
                                "average does not match the arithmetic mean.\r\n");
            TEST_VERIFY_OR_EXIT(verify_raw_rss_averager(&rss_averager) == NS_ERROR_NONE,
                                "test link quality info failed - raw rss averager is incorrect.\r\n");
            PRINTF("add-rss(%4d) sum=%-5d, mean=%-8.2f rss_averager=", rss, sum, mean);
            print_rss_averager(&rss_averager);
        }
    }

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}

ns_error_t
test_link_quality_calculations(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("------------- TEST LINK QUALITY CALCULATIONS\r\n");

    const int8_t rss_list1[] = {-81, -80, -79, -78, -76, -80, -77, -75, -77, -76, -77, -74};
    const struct _rss_test_data rss_data1 = {
        rss_list1,         // mRssList
        sizeof(rss_list1), // mRssListSize
        3                  // mExpectedLinkQuality
    };

    const int8_t rss_list2[] = {-90, -80, -85};
    const struct _rss_test_data rss_data2 = {
        rss_list2,         // mRssList
        sizeof(rss_list2), // mRssListSize
        2                  // mExpectedLinkQuality
    };

    const int8_t rss_list3[] = {-95, -96, -98, -99, -100, -100, -98, -99, -100, -100, -100, -100, -100};
    const struct _rss_test_data rss_data3 = {
        rss_list3,         // mRssList
        sizeof(rss_list3), // mRssListSize
        0                  // mExpectedLinkQuality
    };

    const int8_t rss_list4[] = {-75, -100, -100, -100, -100, -100, -95, -92, -93, -94, -93, -93};
    const struct _rss_test_data rss_data4 = {
        rss_list4,         // mRssList
        sizeof(rss_list4), // mRssListSize
        1                  // mExpectedLinkQuality
    };

    TEST_VERIFY_OR_EXIT(test_link_quality_data(&rss_data1) == NS_ERROR_NONE,
                        "test link quality info failed.\r\n");
    TEST_VERIFY_OR_EXIT(test_link_quality_data(&rss_data2) == NS_ERROR_NONE,
                        "test link quality info failed.\r\n");
    TEST_VERIFY_OR_EXIT(test_link_quality_data(&rss_data3) == NS_ERROR_NONE,
                        "test link quality info failed.\r\n");
    TEST_VERIFY_OR_EXIT(test_link_quality_data(&rss_data4) == NS_ERROR_NONE,
                        "test link quality info failed.\r\n");

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
