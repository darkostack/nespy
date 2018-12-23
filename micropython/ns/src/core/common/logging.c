#include "core/common/logging.h"
#include "core/common/instance.h"

#if (NS_CONFIG_LOG_OUTPUT == NS_CONFIG_LOG_OUTPUT_DEBUG_UART) && \
    (!NS_CONFIG_ENABLE_DEBUG_UART)
#error NS_CONFIG_ENABLE_DEBUG_UART_LOG requires NS_CONFIG_ENABLE_DEBUG_UART
#endif

#define ns_log_dump(format, ...) \
    _ns_dynamic_log(log_level, log_region, format NS_CONFIG_LOG_SUFFIX, ##__VA_ARGS__)

#if NS_CONFIG_LOG_PKT_DUMP
static void
dump_line(ns_log_level_t log_level, ns_log_region_t log_region, const void *abuf, const size_t length)
{
    char buf[80];
    char *cur = buf;

    snprintf(cur, sizeof(buf) - (size_t)(cur - buf), "|");
    cur += strlen(cur);

    for (size_t i = 0; i < 16; i++)
    {
        if (i < length)
        {
            snprintf(cur, sizeof(buf) - (size_t)(cur - buf), " %02X", ((uint8_t *)(abuf))[i]);
            cur += strlen(cur);
        }
        else
        {
            snprintf(cur, sizeof(buf) - (size_t)(cur - buf), " ..");
            cur += strlen(cur);
        }

        if (!((i + 1) % 8))
        {
            snprintf(cur, sizeof(buf) - (size_t)(cur - buf), " |");
            cur += strlen(cur);
        }
    }

    snprintf(cur, sizeof(buf) - (size_t)(cur - buf), " ");
    cur += strlen(cur);

    for (size_t i = 0; i < 16; i++)
    {
        char c = 0x7f & ((char *)(abuf))[i];

        if (i < length && isprint(c))
        {
            snprintf(cur, sizeof(buf) - (size_t)(cur - buf), "%c", c);
            cur += strlen(cur);
        }
        else
        {
            snprintf(cur, sizeof(buf) - (size_t)(cur - buf), ".");
            cur += strlen(cur);
        }
    }

    ns_log_dump("%s", buf);
}

void
ns_dump(ns_log_level_t log_level,
        ns_log_region_t log_region,
        const char *id,
        const void *abuf,
        const size_t length)
{
    size_t idlen = strlen(id);
    const size_t width = 72;
    char buf[80];
    char *cur = buf;

    for (size_t i = 0; i < (width - idlen) / 2 - 5; i++)
    {
        snprintf(cur, sizeof(buf) - (size_t)(cur - buf), "=");
        cur += strlen(cur);
    }

    snprintf(cur, sizeof(buf) - (size_t)(cur - buf), "[%s len=%03u]", id, (uint16_t)(length));
    cur += strlen(cur);

    for (size_t i = 0; i < (width - idlen) / 2 - 4; i++)
    {
        snprintf(cur, sizeof(buf) - (size_t)(cur - buf), "=");
        cur += strlen(cur);
    }

    ns_log_dump("%s", buf);

    for (size_t i = 0; i < length; i += 16)
    {
        dump_line(log_level, log_region, (uint8_t *)(abuf) + i, (length - i) < 16 ? (length - i) : 16);
    }

    cur = buf;

    for (size_t i = 0; i < width; i++)
    {
        snprintf(cur, sizeof(buf) - (size_t)(cur - buf), "-");
        cur += strlen(cur);
    }

    ns_log_dump("%s", buf);
}
#else
void
ns_dump(ns_log_level_t, ns_log_region_t, const char *, const void*, const size_t)
{
}
#endif

const char *
ns_error_to_string(ns_error_t error)
{
    const char *retval;
    switch (error) {
    case NS_ERROR_NONE:
        retval = "ok";
        break;
    case NS_ERROR_FAILED:
        retval = "failed";
        break;
    case NS_ERROR_DROP:
        retval = "drop";
        break;
    case NS_ERROR_NO_BUFS:
        retval = "no-bufs";
        break;
    case NS_ERROR_NO_ROUTE:
        retval = "no-route";
        break;
    case NS_ERROR_BUSY:
        retval = "busy";
        break;
    case NS_ERROR_PARSE:
        retval = "parse";
        break;
    case NS_ERROR_INVALID_ARGS:
        retval = "invalid-args";
        break;
    case NS_ERROR_SECURITY:
        retval = "security";
        break;
    case NS_ERROR_ADDRESS_QUERY:
        retval = "address-query";
        break;
    case NS_ERROR_NO_ADDRESS:
        retval = "no-address";
        break;
    case NS_ERROR_ABORT:
        retval = "abort";
        break;
    case NS_ERROR_NOT_IMPLEMENTED:
        retval = "not-implemented";
        break;
    case NS_ERROR_INVALID_STATE:
        retval = "invalid-state";
        break;
    case NS_ERROR_NO_ACK:
        retval = "no-ack";
        break;
    case NS_ERROR_CHANNEL_ACCESS_FAILURE:
        retval = "channel-access-failure";
        break;
    case NS_ERROR_DETACHED:
        retval = "detached";
        break;
    case NS_ERROR_FCS:
        retval = "fcs-err";
        break;
    case NS_ERROR_NO_FRAME_RECEIVED:
        retval = "no-frame-received";
        break;
    case NS_ERROR_UNKNOWN_NEIGHBOR:
        retval = "unknown-neighbor";
        break;
    case NS_ERROR_INVALID_SOURCE_ADDRESS:
        retval = "invalid-source-address";
        break;
    case NS_ERROR_ADDRESS_FILTERED:
        retval = "address-filtered";
        break;
    case NS_ERROR_DESTINATION_ADDRESS_FILTERED:
        retval = "destination-address-filtered";
        break;
    case NS_ERROR_NOT_FOUND:
        retval = "not-found";
        break;
    case NS_ERROR_ALREADY:
        retval = "already";
        break;
    case NS_ERROR_IP6_ADDRESS_CREATION_FAILURE:
        retval = "ipv6-address-creation-failure";
        break;
    case NS_ERROR_NOT_CAPABLE:
        retval = "not-capable";
        break;
    case NS_ERROR_RESPONSE_TIMEOUT:
        retval = "response-timeout";
        break;
    case NS_ERROR_DUPLICATED:
        retval = "duplicated";
        break;
    case NS_ERROR_REASSEMBLY_TIMEOUT:
        retval = "reassembly-timeout";
        break;
    case NS_ERROR_NOT_TMF:
        retval = "not-tmf";
        break;
    case NS_ERROR_NOT_LOWPAN_DATA_FRAME:
        retval = "non-lowpan-data-frame";
        break;
    case NS_ERROR_DISABLED_FEATURE:
        retval = "disabled-feature";
        break;
    case NS_ERROR_GENERIC:
        retval = "generic-error";
        break;
    case NS_ERROR_LINK_MARGIN_LOW:
        retval = "link-margin-low";
        break;
    default:
        retval = "unknown-error-type";
        break;
    }
    return retval;
}

const char *
ns_log_level_to_prefix_string(ns_log_level_t log_level)
{
    const char *retval = "";

    switch (log_level) {
    case NS_LOG_LEVEL_NONE:
        retval = _NS_LEVEL_NONE_PREFIX;
        break;
    case NS_LOG_LEVEL_CRIT:
        retval = _NS_LEVEL_CRIT_PREFIX;
        break;
    case NS_LOG_LEVEL_WARN:
        retval = _NS_LEVEL_WARN_PREFIX;
        break;
    case NS_LOG_LEVEL_NOTE:
        retval = _NS_LEVEL_NOTE_PREFIX;
        break;
    case NS_LOG_LEVEL_INFO:
        retval = _NS_LEVEL_INFO_PREFIX;
        break;
    case NS_LOG_LEVEL_DEBG:
        retval = _NS_LEVEL_DEBG_PREFIX;
        break;
    }
    return retval;
}

#if NS_CONFIG_LOG_OUTPUT == NS_CONFIG_LOG_OUTPUT_NONE
void
ns_plat_log(ns_log_level_t log_level, ns_log_region_t log_region, const char *format, ...)
{
    (void)log_level;
    (void)log_region;
    (void)format;
}
#endif
