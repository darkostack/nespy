#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/string.h"
#include "ns/sys/core/mac/mac_frame.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>

ns_error_t
test_mac_frame(void)
{
    ns_error_t error = NS_ERROR_NONE;
    printf("----------------------------- TEST MAC FRAME\r\n");

    mac_addr_t mac_addr_test;
    ext_addr_t mac_ext_addr_test;

    mac_ext_addr_gen_random(&mac_ext_addr_test);
    printf("mac random ext addr: %s\r\n",
           string_as_c_string(mac_ext_addr_to_string(&mac_ext_addr_test)));

    mac_addr_make_new(&mac_addr_test);
    mac_addr_set_extended(&mac_addr_test, mac_ext_addr_test);
    TEST_VERIFY_OR_EXIT(mac_addr_type_is_extended(&mac_addr_test),
                        "mac address type does not match as expected\r\n");
    printf("mac addr: %s\r\n", string_as_c_string(mac_addr_to_string(&mac_addr_test)));

    header_ie_t header_ie_test;

    mac_header_ie_init(&header_ie_test);
    mac_header_ie_set_id(&header_ie_test, 0xff);
    mac_header_ie_set_length(&header_ie_test, 0x12);
    printf("IE header          : %04x\r\n", header_ie_test.ie);
    printf("IE header (ID)     : %02x\r\n", mac_header_ie_get_id(&header_ie_test));
    printf("IE header (length) : %02x\r\n", mac_header_ie_get_length(&header_ie_test));

exit:
    return error;
}
