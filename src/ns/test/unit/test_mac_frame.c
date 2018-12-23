#include "ns/error.h"
#include "core/common/instance.h"
#include "core/common/string.h"
#include "core/mac/mac_frame.h"
#include "core/mac/channel_mask.h"
#include "test_util.h"
#include <string.h>
#include <stdio.h>

typedef struct _test_header test_header_t;

struct _test_header {
    uint16_t fcf;
    uint8_t sec_ctl;
    uint8_t header_length;
};

test_header_t test[] = {
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_NONE | MAC_FRAME_FCF_SRC_ADDR_NONE, 0, 3},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_NONE | MAC_FRAME_FCF_SRC_ADDR_SHORT, 0, 7},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_NONE | MAC_FRAME_FCF_SRC_ADDR_EXT, 0, 13},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_NONE, 0, 7},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_EXT | MAC_FRAME_FCF_SRC_ADDR_NONE, 0, 13},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_SHORT, 0, 11},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_EXT, 0, 17},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_EXT | MAC_FRAME_FCF_SRC_ADDR_SHORT, 0, 17},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_EXT | MAC_FRAME_FCF_SRC_ADDR_EXT, 0, 23},

    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_SHORT |
        MAC_FRAME_FCF_PANID_COMPRESSION,
     0, 9},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_EXT |
        MAC_FRAME_FCF_PANID_COMPRESSION,
     0, 15},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_EXT | MAC_FRAME_FCF_SRC_ADDR_SHORT |
        MAC_FRAME_FCF_PANID_COMPRESSION,
     0, 15},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_EXT | MAC_FRAME_FCF_SRC_ADDR_EXT |
        MAC_FRAME_FCF_PANID_COMPRESSION,
     0, 21},

    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_SHORT |
        MAC_FRAME_FCF_PANID_COMPRESSION | MAC_FRAME_FCF_SECURITY_ENABLED,
     MAC_FRAME_SEC_MIC_32 | MAC_FRAME_KEY_ID_MODE_1, 15},
    {MAC_FRAME_FCF_FRAME_VERSION_2006 | MAC_FRAME_FCF_DST_ADDR_SHORT | MAC_FRAME_FCF_SRC_ADDR_SHORT |
        MAC_FRAME_FCF_PANID_COMPRESSION | MAC_FRAME_FCF_SECURITY_ENABLED,
     MAC_FRAME_SEC_MIC_32 | MAC_FRAME_KEY_ID_MODE_2, 19},
};

ns_error_t
test_mac_header(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("---------------------------- TEST MAC HEADER\r\n");

    for (unsigned i = 0; i < NS_ARRAY_LENGTH(test); i++) {
        uint8_t psdu[MAC_FRAME_MTU];
        mac_frame_t frame;
        frame.psdu = psdu;
        mac_frame_init_mac_header(&frame, test[i].fcf, test[i].sec_ctl);
        TEST_VERIFY_OR_EXIT(mac_frame_get_header_length(&frame) == test[i].header_length,
                            "mac header test failed.\r\n");
    }

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}

static ns_error_t
verify_channel_mask_content(mac_channel_mask_t *channel_mask, uint8_t *channels, uint8_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t index = 0;
    uint8_t channel;

    for (channel = NS_RADIO_CHANNEL_MIN; channel <= NS_RADIO_CHANNEL_MAX; channel++) {
        if (index < length) {
            if (channel == channels[index]) {
                index++;
                TEST_VERIFY_OR_EXIT(mac_channel_mask_contains_channel(channel_mask, channel),
                                    "channel mask contains channel failed.\r\n");
            } else {
                TEST_VERIFY_OR_EXIT(!mac_channel_mask_contains_channel(channel_mask, channel),
                                    "channel mask contains channel failed.\r\n");
            }
        }
    }

    index = 0;
    channel = MAC_CHANNEL_MASK_CHANNEL_ITERATOR_FIRST;

    while (mac_channel_mask_get_next_channel(channel_mask, &channel) == NS_ERROR_NONE) {
        TEST_VERIFY_OR_EXIT(channel == channels[index++],
                            "channel mask get next failed.\r\n");
    }

    TEST_VERIFY_OR_EXIT(index == length,
                        "channel mask get next channel failed.\r\n");

    if (length == 1) { 
        TEST_VERIFY_OR_EXIT(mac_channel_mask_is_single_channel(channel_mask),
                            "channel mask is single channel failed.\r\n");
    } else {
        TEST_VERIFY_OR_EXIT(!mac_channel_mask_is_single_channel(channel_mask),
                            "channel mask is single channel failed.\r\n");
    }

    TEST_VERIFY_OR_EXIT(length == mac_channel_mask_get_number_of_channels(channel_mask),
                        "channel mask get number of channels failed.\r\n");
exit:
    return error;
}

ns_error_t
test_mac_channel_mask(void)
{
    ns_error_t error = NS_ERROR_NONE;
    uint8_t all_channels[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
    uint8_t channels1[]    = {11, 14, 15, 16, 17, 20, 21, 22, 24, 25};
    uint8_t channels2[]    = {14, 21, 26};
    uint8_t channels3[]    = {14, 21};
    uint8_t channels4[]    = {20};

    mac_channel_mask_t mask1;
    mac_channel_mask_ctor(&mask1, 0);

    mac_channel_mask_t mask2;
    mac_channel_mask_ctor(&mask2, NS_RADIO_SUPPORTED_CHANNELS);

    printf("---------------------- TEST MAC CHANNEL MASK\r\n");

    TEST_VERIFY_OR_EXIT(mac_channel_mask_is_empty(&mask1), "channel mask is empty failed.\r\n");
    //printf("empty = %s\r\n", string_as_c_string(mac_channel_mask_to_string(&mask1)));

    TEST_VERIFY_OR_EXIT(!mac_channel_mask_is_empty(&mask2), "channel mask is empty failed.\r\n");
    TEST_VERIFY_OR_EXIT(mac_channel_mask_get_mask(&mask2) == NS_RADIO_SUPPORTED_CHANNELS,
                        "channel mask get mask failed.\r\n");
    //printf("all_channels = %s\r\n", string_as_c_string(mac_channel_mask_to_string(&mask2)));

    mac_channel_mask_set_mask(&mask1, NS_RADIO_SUPPORTED_CHANNELS);
    TEST_VERIFY_OR_EXIT(!mac_channel_mask_is_empty(&mask1), "channel mask is empty failed.\r\n");
    TEST_VERIFY_OR_EXIT(mac_channel_mask_get_mask(&mask1) == NS_RADIO_SUPPORTED_CHANNELS,
                        "channel mask get mask failed.\r\n");

    verify_channel_mask_content(&mask1, all_channels, sizeof(all_channels));

    // test channel mask remove channel
    for (uint8_t index = 0; index < sizeof(all_channels) - 1; index++) {
        mac_channel_mask_remove_channel(&mask1, all_channels[index]);
        verify_channel_mask_content(&mask1, &all_channels[index + 1], sizeof(all_channels) - 1 - index);
        //printf("all_channels = %s\r\n", string_as_c_string(mac_channel_mask_to_string(&mask1)));
    }

    mac_channel_mask_clear(&mask1);
    TEST_VERIFY_OR_EXIT(mac_channel_mask_is_empty(&mask1), "channel mask is empty failed.\r\n");
    verify_channel_mask_content(&mask1, NULL, 0);

    for (uint16_t index = 0; index < sizeof(channels1); index++) {
        mac_channel_mask_add_channel(&mask1, channels1[index]);
    }

    //printf("channels1 = %s\r\n", string_as_c_string(mac_channel_mask_to_string(&mask1)));

    TEST_VERIFY_OR_EXIT(!mac_channel_mask_is_empty(&mask1), "channel mask is empty failed.\r\n");
    verify_channel_mask_content(&mask1, channels1, sizeof(channels1));

    mac_channel_mask_clear(&mask2);

    for (uint16_t index = 0; index < sizeof(channels2); index++) {
        mac_channel_mask_add_channel(&mask2, channels2[index]);
    }

    //printf("channels2 = %s\r\n", string_as_c_string(mac_channel_mask_to_string(&mask2)));

    TEST_VERIFY_OR_EXIT(!mac_channel_mask_is_empty(&mask2), "channel mask is empty failed.\r\n");
    verify_channel_mask_content(&mask2, channels2, sizeof(channels2));

    mac_channel_mask_intersect(&mask1, &mask2);
    verify_channel_mask_content(&mask1, channels3, sizeof(channels3));

    mac_channel_mask_clear(&mask2);
    mac_channel_mask_add_channel(&mask2, channels4[0]);
    verify_channel_mask_content(&mask2, channels4, sizeof(channels4));

    //printf("channels4 = %s\r\n", string_as_c_string(mac_channel_mask_to_string(&mask2)));

    mac_channel_mask_clear(&mask1);
    mac_channel_mask_clear(&mask2);
    TEST_VERIFY_OR_EXIT(mac_channel_mask_is_equal(&mask1, &mask2), "channel mask is equal failed.\r\n");

    mac_channel_mask_set_mask(&mask1, NS_RADIO_SUPPORTED_CHANNELS);
    mac_channel_mask_set_mask(&mask2, NS_RADIO_SUPPORTED_CHANNELS);
    TEST_VERIFY_OR_EXIT(mac_channel_mask_is_equal(&mask1, &mask2), "channel mask is equal failed.\r\n");

    mac_channel_mask_clear(&mask1);
    TEST_VERIFY_OR_EXIT(!mac_channel_mask_is_equal(&mask1, &mask2), "channel mask is equal failed.\r\n");

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
