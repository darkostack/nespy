#include "ns/include/platform/misc.h"
#include <stdio.h>
#include <stdlib.h>

#define STR_COLOR_RED "\x1B[31m"
#define STR_COLOR_NRM "\x1B[0m"

void
ns_plat_assert_fail(const char *filename, int line_number)
{
    printf("%sASSERT FAILED%s: %s: %d\r\n", STR_COLOR_RED, STR_COLOR_NRM, filename, line_number);
    exit(EXIT_FAILURE);
}
