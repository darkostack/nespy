#ifndef NS_PLATFORM_LOGGING_H_
#define NS_PLATFORM_LOGGING_H_

#include <stdarg.h>
#include <stdint.h>

#define NS_LOG_LEVEL_NONE 0
#define NS_LOG_LEVEL_CRIT 1
#define NS_LOG_LEVEL_WARN 2
#define NS_LOG_LEVEL_NOTE 3
#define NS_LOG_LEVEL_INFO 4
#define NS_LOG_LEVEL_DEBG 5

typedef uint8_t ns_log_level_t;

typedef enum _ns_log_region {
    NS_LOG_REGION_API      = 1,  // API
    NS_LOG_REGION_MLE      = 2,  // MLE
    NS_LOG_REGION_ARP      = 3,  // EID-to-RLOC mapping.
    NS_LOG_REGION_NET_DATA = 4,  // Network Data
    NS_LOG_REGION_ICMP     = 5,  // ICMPv6
    NS_LOG_REGION_IP6      = 6,  // IPv6
    NS_LOG_REGION_MAC      = 7,  // IEEE 802.15.4 MAC
    NS_LOG_REGION_MEM      = 8,  // Memory
    NS_LOG_REGION_NCP      = 9,  // NCP
    NS_LOG_REGION_MESH_COP = 10, // Mesh Commissioning Protocol
    NS_LOG_REGION_NET_DIAG = 11, // Network Diagnostic
    NS_LOG_REGION_PLATFORM = 12, // Platform
    NS_LOG_REGION_COAP     = 13, // CoAP
    NS_LOG_REGION_CLI      = 14, // CLI
    NS_LOG_REGION_CORE     = 15, // Core
    NS_LOG_REGION_UTIL     = 16, // Utility module
} ns_log_region_t;

void
ns_plat_log(ns_log_level_t log_level, ns_log_region_t log_region, const char *format, ...);

void
ns_plat_logv(ns_log_level_t log_level, ns_log_region_t log_region, const char *format, va_list ap);

#endif // NS_PLATFORM_LOGGING_H_
