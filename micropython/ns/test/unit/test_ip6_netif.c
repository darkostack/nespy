#include "ns/error.h"
#include "core/common/instance.h"
#include "core/common/string.h"
#include "core/mac/mac_frame.h"
#include "core/net/ip6_address.h"
#include "core/net/netif.h"
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
test_ip6_netif(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("----------------------------- TEST IP6 NETIF\r\n");

    instance_t *instance = instance_get();

    ip6_netif_t netif;
    mac_ext_addr_t ext_addr[8];
    ip6_addr_t ip6_addr[NS_ARRAY_LENGTH(ext_addr)];
    ip6_netif_unicast_addr_t ip6_unicast_addr[NS_ARRAY_LENGTH(ext_addr)];
    ip6_netif_multicast_addr_t ip6_multicast_addr[NS_ARRAY_LENGTH(ext_addr)];

    for (int i = 0; i < NS_ARRAY_LENGTH(ext_addr); i++) {
        mac_ext_addr_generate_random(&ext_addr[i]);
        PRINTF("ext_addr %d: %s\r\n", i, string_as_c_string(mac_ext_addr_to_string(&ext_addr[i])));

        ip6_addr_clear(&ip6_addr[i]);
        ip6_addr_set_iid_from_mac_ext_addr(&ip6_addr[i], &ext_addr[i]);
        PRINTF("ip6_addr: %s\r\n", string_as_c_string(ip6_addr_to_string(&ip6_addr[i])));

        ip6_unicast_addr[i].addr = ip6_addr[i];
        ip6_unicast_addr[i].rloc = false;

        ip6_multicast_addr[i].addr = ip6_addr[i];
    }

    ip6_netif_ctor(&netif, instance, NS_NETIF_INTERFACE_ID_THREAD);

    // --- add unicast address
    TEST_VERIFY_OR_EXIT(ip6_netif_add_unicast_addr(&netif, &ip6_unicast_addr[0]) == NS_ERROR_NONE,
                        "ip6 netif test failed - add unicast address.\r\n");

    TEST_VERIFY_OR_EXIT(ip6_netif_add_unicast_addr(&netif, &ip6_unicast_addr[0]) == NS_ERROR_ALREADY,
                        "ip6 netif test failed - add unicast address should return already.\r\n");

    TEST_VERIFY_OR_EXIT(ip6_netif_is_unicast_addr(&netif, (const ip6_addr_t *)&ip6_addr[0]) == true,
                        "ip6 netif test failed - return to true for is unicast address.\r\n");

    // --- remove unicast address
    TEST_VERIFY_OR_EXIT(ip6_netif_remove_unicast_addr(&netif, &ip6_unicast_addr[0]) == NS_ERROR_NONE,
                        "ip6 netif test failed - remove unicast address.\r\n");

    TEST_VERIFY_OR_EXIT(ip6_netif_is_unicast_addr(&netif, (const ip6_addr_t *)&ip6_addr[0]) == false,
                        "ip6 netif test failed - return to false for isn't unicast address.\r\n");

    // --- add external unicast address
    TEST_VERIFY_OR_EXIT(ip6_netif_add_external_unicast_addr(&netif, &ip6_unicast_addr[0]) == NS_ERROR_NONE,
                        "ip6 netif test failed - to add external unicast addr.\r\n");

    TEST_VERIFY_OR_EXIT(ip6_netif_add_external_unicast_addr(&netif, &ip6_unicast_addr[1]) == NS_ERROR_NONE,
                        "ip6 netif test failed - to add external unicast addr.\r\n");

    TEST_VERIFY_OR_EXIT(ip6_netif_add_external_unicast_addr(&netif, &ip6_unicast_addr[2]) == NS_ERROR_NONE,
                        "ip6 netif test failed - to add external unicast addr.\r\n");

    TEST_VERIFY_OR_EXIT(ip6_netif_add_external_unicast_addr(&netif, &ip6_unicast_addr[3]) == NS_ERROR_NONE,
                        "ip6 netif test failed - to add external unicast addr.\r\n");

    // --- test ip6 netif error case
    error = ip6_netif_add_external_unicast_addr(&netif, &ip6_unicast_addr[4]);

    PRINTF("error: %s\r\n", ns_error_to_string(error));

    VERIFY_OR_EXIT(error == NS_ERROR_NO_BUFS);

    error = NS_ERROR_NONE;

    // --- remove external unicast address
    TEST_VERIFY_OR_EXIT(ip6_netif_remove_external_unicast_addr(&netif, &ip6_unicast_addr[3].addr) == NS_ERROR_NONE,
                        "ip6 netif test failed - to remove external unicast addr.\r\n");

    TEST_VERIFY_OR_EXIT(ip6_netif_add_external_unicast_addr(&netif, &ip6_unicast_addr[4]) == NS_ERROR_NONE,
                        "ip6 netif test failed - to add external unicast addr.\r\n");

    error = ip6_netif_remove_external_unicast_addr(&netif, &ip6_unicast_addr[3].addr);

    PRINTF("error: %s\r\n", ns_error_to_string(error));

    VERIFY_OR_EXIT(error == NS_ERROR_NOT_FOUND);

    error = NS_ERROR_NONE;

    ip6_netif_remove_all_external_unicast_addrs(&netif);

    error = ip6_netif_remove_external_unicast_addr(&netif, &ip6_unicast_addr[0].addr);
    VERIFY_OR_EXIT(error == NS_ERROR_NOT_FOUND);

    error = ip6_netif_remove_external_unicast_addr(&netif, &ip6_unicast_addr[1].addr);
    VERIFY_OR_EXIT(error == NS_ERROR_NOT_FOUND);

    error = ip6_netif_remove_external_unicast_addr(&netif, &ip6_unicast_addr[2].addr);
    VERIFY_OR_EXIT(error == NS_ERROR_NOT_FOUND);

    error = ip6_netif_remove_external_unicast_addr(&netif, &ip6_unicast_addr[4].addr);
    VERIFY_OR_EXIT(error == NS_ERROR_NOT_FOUND);

    error = NS_ERROR_NONE;

    // --- test external multicast subscribe
    error = ip6_netif_subscribe_external_multicast(&netif, (const ip6_addr_t *)&ip6_addr[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = ip6_netif_subscribe_external_multicast(&netif, (const ip6_addr_t *)&ip6_addr[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // --- test error case for external multicast subscribe
    error = ip6_netif_subscribe_external_multicast(&netif, (const ip6_addr_t *)&ip6_addr[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NO_BUFS);

    error = NS_ERROR_NONE;

    // --- test external multicast get next
    uint8_t iterator = 0;
    ip6_addr_t external_ip6_addr[NS_CONFIG_MAX_EXT_MULTICAST_IP_ADDRS];

    for (int i = 0; i < NS_ARRAY_LENGTH(external_ip6_addr); i++) {
        ip6_addr_clear(&external_ip6_addr[i]);

        error = ip6_netif_get_next_external_multicast(&netif, &iterator, &external_ip6_addr[i]);
        VERIFY_OR_EXIT(error == NS_ERROR_NONE);

        PRINTF("external ip6 addr %u: %s\r\n", iterator,
               string_as_c_string(ip6_addr_to_string(&external_ip6_addr[i])));

        VERIFY_OR_EXIT(ip6_addr_is_equal(&external_ip6_addr[i], &ip6_addr[i]), error = NS_ERROR_FAILED);
    }

    // --- test multicast is subscribed
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, &ip6_addr[0]) == true, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, &ip6_addr[1]) == true, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, &ip6_addr[2]) == false, error = NS_ERROR_FAILED);

    // --- test external unsubscribe multicast
    error = ip6_netif_unsubscribe_external_multicast(&netif, &external_ip6_addr[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, &external_ip6_addr[0]) == false,
                   error = NS_ERROR_FAILED);

    ip6_netif_unsubscribe_all_external_multicast(&netif);

    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, &external_ip6_addr[0]) == false,
                   error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, &external_ip6_addr[1]) == false,
                   error = NS_ERROR_FAILED);

    // --- test external multicast unsubscibed error case
    error = ip6_netif_unsubscribe_external_multicast(&netif, &external_ip6_addr[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NOT_FOUND);

    error = NS_ERROR_NONE;

    // --- test multicast subscribe
    VERIFY_OR_EXIT((error = ip6_netif_subscribe_multicast(&netif, &ip6_multicast_addr[0])) == NS_ERROR_NONE);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[0].addr) == true, error = NS_ERROR_FAILED);

    VERIFY_OR_EXIT((error = ip6_netif_subscribe_multicast(&netif, &ip6_multicast_addr[1])) == NS_ERROR_NONE);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[1].addr) == true, error = NS_ERROR_FAILED);

    VERIFY_OR_EXIT((error = ip6_netif_subscribe_multicast(&netif, &ip6_multicast_addr[2])) == NS_ERROR_NONE);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[2].addr) == true, error = NS_ERROR_FAILED);


    // --- test multicast unsubscribe
    VERIFY_OR_EXIT((error = ip6_netif_unsubscribe_multicast(&netif, &ip6_multicast_addr[0])) == NS_ERROR_NONE);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[0].addr) == false, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[1].addr) == true, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[2].addr) == true, error = NS_ERROR_FAILED);

    VERIFY_OR_EXIT((error = ip6_netif_unsubscribe_multicast(&netif, &ip6_multicast_addr[1])) == NS_ERROR_NONE);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[0].addr) == false, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[1].addr) == false, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[2].addr) == true, error = NS_ERROR_FAILED);

    VERIFY_OR_EXIT((error = ip6_netif_unsubscribe_multicast(&netif, &ip6_multicast_addr[2])) == NS_ERROR_NONE);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[0].addr) == false, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[1].addr) == false, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(ip6_netif_is_multicast_subscribed(&netif, (const ip6_addr_t *)&ip6_multicast_addr[2].addr) == false, error = NS_ERROR_FAILED);

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
