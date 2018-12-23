#include "platform-unix.h"
#include "ns/config.h"
#include "ns/platform/logging.h"
#include "core/common/code_utils.h"
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

// Macro to append content to end of the log string.

#define LOG_PRINTF(...)                                                                      \
    chars_written = snprintf(&log_string[offset], sizeof(log_string) - offset, __VA_ARGS__); \
    EXPECT_ACTION(chars_written >= 0, log_string[offset] = 0);                               \
    offset += (unsigned int)chars_written;                                                   \
    EXPECT_ACTION(offset < sizeof(log_string), log_string[sizeof(log_string) - 1] = 0)

#if (NS_CONFIG_LOG_OUTPUT == NS_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED) || \
    (NS_CONFIG_LOG_OUTPUT == NS_CONFIG_LOG_OUTPUT_NCP_SPINEL)
void
ns_plat_log(ns_log_level_t log_level, ns_log_region_t log_region, const char *format, ...)
{
    (void)log_level;
    (void)log_region;

    char log_string[512];
    unsigned int offset;
    int chars_written;
    va_list args;

    offset = 0;

    LOG_PRINTF("[%d] ", g_node_id);

    va_start(args, format);
    chars_written = vsnprintf(&log_string[offset], sizeof(log_string) - offset, format, args);
    va_end(args);

    EXPECT_ACTION(chars_written >= 0, log_string[offset] = 0);

exit:
    printf("%s\r\n", log_string);
}
#endif
