#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/string.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>

STRING(info, 16);

ns_error_t
test_string(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("-------------------------------- TEST STRING\r\n");

    TEST_VERIFY_OR_EXIT(string_set(&info_string, "info: %d", 10) == NS_ERROR_NONE,
                        "string set failed.\r\n");
    TEST_VERIFY_OR_EXIT(strcmp(string_as_c_string(&info_string), "info: 10") == 0,
                        "string content does not match as expected.\r\n");
    TEST_VERIFY_OR_EXIT(string_get_length(&info_string) == 8,
                        "string length does not match as expected.\r\n");

    string_clear(&info_string);

    TEST_VERIFY_OR_EXIT(string_get_length(&info_string) == 0,
                        "string length is not zero after clear\r\n");

    // test the error case
    TEST_VERIFY_OR_EXIT(string_set(&info_string, "info dump 123456789") == NS_ERROR_NO_BUFS,
                        "string does not failed as expected.\r\n");

    // Note: in case of no bufs, string set will set the length to string.bufsize - 1
    TEST_VERIFY_OR_EXIT(string_get_length(&info_string) == info_string.bufsize - 1,
                        "string size does not match as expected.\r\n");
    TEST_VERIFY_OR_EXIT(strcmp(string_as_c_string(&info_string), "info dump 12345") == 0,
                        "string content does not match as expected.\r\n");

    string_clear(&info_string);

    TEST_VERIFY_OR_EXIT(string_get_length(&info_string) == 0,
                        "string length is not zero after clear\r\n");

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
