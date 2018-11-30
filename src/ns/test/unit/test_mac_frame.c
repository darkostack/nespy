#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/string.h"
#include "ns/sys/core/mac/mac_frame.h"
#include "ns/test/unit/test_util.h"
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
