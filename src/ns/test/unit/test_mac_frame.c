#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/string.h"
#include "ns/sys/core/mac/mac_frame.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>

mac_addr_t mac_addr_test;
ext_addr_t mac_ext_addr_test;

ns_error_t
test_mac_frame(void)
{
    ns_error_t error = NS_ERROR_NONE;
    printf("----------------------------- TEST MAC FRAME\r\n");
    mac_ext_addr_gen_random(&mac_ext_addr_test);
    printf("mac random ext addr: %s\r\n",
           string_as_c_string(mac_ext_addr_to_string(&mac_ext_addr_test)));

    mac_addr_make_new(&mac_addr_test);
    mac_addr_set_extended(&mac_addr_test, mac_ext_addr_test);
    TEST_VERIFY_OR_EXIT(mac_addr_type_is_extended(&mac_addr_test),
                        "mac address type does not match as expected\r\n");
    printf("mac addr: %s\r\n", string_as_c_string(mac_addr_to_string(&mac_addr_test)));
exit:
    return error;
}
