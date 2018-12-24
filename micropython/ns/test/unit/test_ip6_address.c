#include "ns/error.h"
#include "core/common/instance.h"
#include "core/common/string.h"
#include "core/mac/mac_frame.h"
#include "core/net/ip6_address.h"
#include "test_util.h"
#include <string.h>
#include <stdio.h>

#define PRINT_DEBUG 0

#if PRINT_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

ns_error_t
test_ip6_address(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("--------------------------- TEST IP6 ADDRESS\r\n");

    mac_ext_addr_t ext_addr;
    mac_ext_addr_t ext_addr2;
    ip6_addr_t ip6_addr;
    ip6_addr_t ip6_addr2;

    mac_ext_addr_generate_random(&ext_addr);

    PRINTF("ext_addr: %s\r\n", string_as_c_string(mac_ext_addr_to_string(&ext_addr)));

    ip6_addr_clear(&ip6_addr);
    ip6_addr_set_iid_from_mac_ext_addr(&ip6_addr, &ext_addr);

    PRINTF("ip6_addr: %s\r\n", string_as_c_string(ip6_addr_to_string(&ip6_addr)));

    ip6_addr_clear(&ip6_addr2);
    ip6_addr_from_string(&ip6_addr2, string_as_c_string(ip6_addr_to_string(&ip6_addr)));

    PRINTF("ip6_addr: %s\r\n", string_as_c_string(ip6_addr_to_string(&ip6_addr2)));

    TEST_VERIFY_OR_EXIT(strcmp(string_as_c_string(ip6_addr_to_string(&ip6_addr)),
                               string_as_c_string(ip6_addr_to_string(&ip6_addr2))) == 0,
                        "ip6 address test failed - ip6 address doesn't match\r\n");

    ip6_addr_to_ext_addr(&ip6_addr, &ext_addr2);

    PRINTF("ext_addr: %s\r\n", string_as_c_string(mac_ext_addr_to_string(&ext_addr2)));

    TEST_VERIFY_OR_EXIT(strcmp(string_as_c_string(mac_ext_addr_to_string(&ext_addr)),
                               string_as_c_string(mac_ext_addr_to_string(&ext_addr2))) == 0,
                        "ip6 address test failed - ext address doesn't match\r\n");

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
