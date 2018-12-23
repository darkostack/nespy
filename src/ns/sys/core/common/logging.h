#ifndef NS_CORE_COMMON_LOGGING_H_
#define NS_CORE_COMMON_LOGGING_H_

#include "ns/sys/core/core-config.h"
#include "ns/include/logging.h"
#include "ns/include/platform/logging.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if NS_CONFIG_LOG_PREPEND_LEVEL
#define _NS_LEVEL_NONE_PREFIX "[NONE]"
#define _NS_LEVEL_CRIT_PREFIX "[CRIT]"
#define _NS_LEVEL_WARN_PREFIX "[WARN]"
#define _NS_LEVEL_NOTE_PREFIX "[NOTE]"
#define _NS_LEVEL_INFO_PREFIX "[INFO]"
#define _NS_LEVEL_DEBG_PREFIX "[DEBG]"
#define _NS_REGION_SUFFIX ": "
#else
#define _NS_LEVEL_NONE_PREFIX ""
#define _NS_LEVEL_CRIT_PREFIX ""
#define _NS_LEVEL_WARN_PREFIX ""
#define _NS_LEVEL_NOTE_PREFIX ""
#define _NS_LEVEL_INFO_PREFIX ""
#define _NS_LEVEL_DEBG_PREFIX ""
#define _NS_REGION_SUFFIX
#endif

#if NS_CONFIG_LOG_PREPEND_REGION
#define _NS_REGION_API_PREFIX "-API-----: "
#define _NS_REGION_MLE_PREFIX "-MLE-----: "
#define _NS_REGION_ARP_PREFIX "-ARP-----: "
#define _NS_REGION_NET_DATA_PREFIX "-N-DATA--: "
#define _NS_REGION_ICMP_PREFIX "-ICMP----: "
#define _NS_REGION_IP6_PREFIX "-IP6-----: "
#define _NS_REGION_MAC_PREFIX "-MAC-----: "
#define _NS_REGION_MEM_PREFIX "-MEM-----: "
#define _NS_REGION_NCP_PREFIX "-NCP-----: "
#define _NS_REGION_MESH_COP_PREFIX "-MESH-CP-: "
#define _NS_REGION_NET_DIAG_PREFIX "-DIAG----: "
#define _NS_REGION_PLATFORM_PREFIX "-PLAT----: "
#define _NS_REGION_COAP_PREFIX "-COAP----: "
#define _NS_REGION_CLI_PREFIX "-CLI-----: "
#define _NS_REGION_CORE_PREFIX "-CORE----: "
#define _NS_REGION_UTIL_PREFIX "-UTIL----: "
#else
#define _NS_REGION_API_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_MLE_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_ARP_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_NET_DATA_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_ICMP_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_IP6_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_MAC_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_MEM_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_NCP_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_MESH_COP_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_NET_DIAG_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_PLATFORM_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_COAP_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_CLI_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_CORE_PREFIX _NS_REGION_SUFFIX
#define _NS_REGION_UTIL_PREFIX _NS_REGION_SUFFIX
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_CRIT
#define ns_log_crit(region, format, ...) \
    _ns_log_formatter(NS_LOG_LEVEL_CRIT, region, _NS_LEVEL_CRIT_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit(region, format, ...)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_WARN
#define ns_log_warn(region, format, ...) \
    _ns_log_formatter(NS_LOG_LEVEL_WARN, region, _NS_LEVEL_WARN_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_warn(region, format, ...)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_NOTE
#define ns_log_note(region, format, ...) \
    _ns_log_formatter(NS_LOG_LEVEL_NOTE, region, _NS_LEVEL_NOTE_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_note(region, format, ...)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO
#define ns_log_info(region, format, ...) \
    _ns_log_formatter(NS_LOG_LEVEL_INFO, region, _NS_LEVEL_INFO_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_info(region, format, ...)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_DEBG
#define ns_log_debg(region, format, ...) \
    _ns_log_formatter(NS_LOG_LEVEL_DEBG, region, _NS_LEVEL_DEBG_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_debg(region, format, ...)
#endif

#if NS_CONFIG_LOG_API == 1
#define ns_log_crit_api(format, ...) ns_log_crit(NS_LOG_REGION_API, _NS_REGION_API_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_api(format, ...) ns_log_warn(NS_LOG_REGION_API, _NS_REGION_API_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_api(format, ...) ns_log_note(NS_LOG_REGION_API, _NS_REGION_API_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_api(format, ...) ns_log_info(NS_LOG_REGION_API, _NS_REGION_API_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_api(format, ...) ns_log_debg(NS_LOG_REGION_API, _NS_REGION_API_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_api(format, ...)
#define ns_log_warn_api(format, ...)
#define ns_log_note_api(format, ...)
#define ns_log_info_api(format, ...)
#define ns_log_debg_api(format, ...)
#endif

#if NS_CONFIG_LOG_MLE == 1
#define ns_log_crit_mesh_cop(format, ...) \
    ns_log_crit(NS_LOG_REGION_MESH_COP, _NS_REGION_MESH_COP_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_mesh_cop(format, ...) \
    ns_log_warn(NS_LOG_REGION_MESH_COP, _NS_REGION_MESH_COP_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_mesh_cop(format, ...) \
    ns_log_note(NS_LOG_REGION_MESH_COP, _NS_REGION_MESH_COP_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_mesh_cop(format, ...) \
    ns_log_info(NS_LOG_REGION_MESH_COP, _NS_REGION_MESH_COP_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_mesh_cop(format, ...) \
    ns_log_debg(NS_LOG_REGION_MESH_COP, _NS_REGION_MESH_COP_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_mesh_cop(format, ...)
#define ns_log_warn_mesh_cop(format, ...)
#define ns_log_note_mesh_cop(format, ...)
#define ns_log_info_mesh_cop(format, ...)
#define ns_log_debg_mesh_cop(format, ...)
#endif

#define ns_log_crit_mbedtls(format, ...) ns_log_crit_mesh_cop(format, ##__VA_ARGS__)
#define ns_log_warn_mbedtls(format, ...) ns_log_warn_mesh_cop(format, ##__VA_ARGS__)
#define ns_log_note_mbedtls(format, ...) ns_log_note_mesh_cop(format, ##__VA_ARGS__)
#define ns_log_info_mbedtls(format, ...) ns_log_info_mesh_cop(format, ##__VA_ARGS__)
#define ns_log_debg_mbedtls(format, ...) ns_log_debg_mesh_cop(format, ##__VA_ARGS__)

#if NS_CONFIG_LOG_MLE == 1
#define ns_log_crit_mle(format, ...) ns_log_crit(NS_LOG_REGION_MLE, _NS_REGION_MLE_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_mle(format, ...) ns_log_warn(NS_LOG_REGION_MLE, _NS_REGION_MLE_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_mle_err(error, format, ...)                                                          \
    ns_log_warn(NS_LOG_REGION_MLE, _NS_REGION_MLE_PREFIX "error %s: " format, ns_error_to_string(error), \
              ##__VA_ARGS__)
#define ns_log_note_mle(format, ...) ns_log_note(NS_LOG_REGION_MLE, _NS_REGION_MLE_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_mle(format, ...) ns_log_info(NS_LOG_REGION_MLE, _NS_REGION_MLE_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_mle(format, ...) ns_log_debg(NS_LOG_REGION_MLE, _NS_REGION_MLE_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_mle(format, ...)
#define ns_log_warn_mle(format, ...)
#define ns_log_warn_mle_err(error, format, ...)
#define ns_log_note_mle(format, ...)
#define ns_log_info_mle(format, ...)
#define ns_log_debg_mle(format, ...)
#endif

#if NS_CONFIG_LOG_ARP == 1
#define ns_log_crit_arp(format, ...) ns_log_crit(NS_LOG_REGION_ARP, _NS_REGION_ARP_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_arp(format, ...) ns_log_warn(NS_LOG_REGION_ARP, _NS_REGION_ARP_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_arp(format, ...) ns_log_note(NS_LOG_REGION_ARP, _NS_REGION_ARP_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_arp(format, ...) ns_log_info(NS_LOG_REGION_ARP, _NS_REGION_ARP_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_arp(format, ...) ns_log_debg(NS_LOG_REGION_ARP, _NS_REGION_ARP_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_arp(format, ...)
#define ns_log_warn_arp(format, ...)
#define ns_log_note_arp(format, ...)
#define ns_log_info_arp(format, ...)
#define ns_log_debg_arp(format, ...)
#endif

#if NS_CONFIG_LOG_NETDATA == 1
#define ns_log_crit_net_data(format, ...) \
    ns_log_crit(NS_LOG_REGION_NET_DATA, _NS_REGION_NET_DATA_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_net_data(format, ...) \
    ns_log_warn(NS_LOG_REGION_NET_DATA, _NS_REGION_NET_DATA_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_net_data(format, ...) \
    ns_log_note(NS_LOG_REGION_NET_DATA, _NS_REGION_NET_DATA_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_net_data(format, ...) \
    ns_log_info(NS_LOG_REGION_NET_DATA, _NS_REGION_NET_DATA_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_net_data(format, ...) \
    ns_log_debg(NS_LOG_REGION_NET_DATA, _NS_REGION_NET_DATA_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_net_data(format, ...)
#define ns_log_warn_net_data(format, ...)
#define ns_log_note_net_data(format, ...)
#define ns_log_info_net_data(format, ...)
#define ns_log_debg_net_data(format, ...)
#endif

#if NS_CONFIG_LOG_ICMP == 1
#define ns_log_crit_icmp(format, ...) ns_log_crit(NS_LOG_REGION_ICMP, _NS_REGION_ICMP_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_icmp(format, ...) ns_log_warn(NS_LOG_REGION_ICMP, _NS_REGION_ICMP_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_icmp(format, ...) ns_log_note(NS_LOG_REGION_ICMP, _NS_REGION_ICMP_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_icmp(format, ...) ns_log_info(NS_LOG_REGION_ICMP, _NS_REGION_ICMP_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_icmp(format, ...) ns_log_debg(NS_LOG_REGION_ICMP, _NS_REGION_ICMP_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_icmp(format, ...)
#define ns_log_warn_icmp(format, ...)
#define ns_log_note_icmp(format, ...)
#define ns_log_info_icmp(format, ...)
#define ns_log_debg_icmp(format, ...)
#endif

#if NS_CONFIG_LOG_IP6 == 1
#define ns_log_crit_ip6(format, ...) ns_log_crit(NS_LOG_REGION_IP6, _NS_REGION_IP6_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_ip6(format, ...) ns_log_warn(NS_LOG_REGION_IP6, _NS_REGION_IP6_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_ip6(format, ...) ns_log_note(NS_LOG_REGION_IP6, _NS_REGION_IP6_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_ip6(format, ...) ns_log_info(NS_LOG_REGION_IP6, _NS_REGION_IP6_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_ip6(format, ...) ns_log_debg(NS_LOG_REGION_IP6, _NS_REGION_IP6_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_ip6(format, ...)
#define ns_log_warn_ip6(format, ...)
#define ns_log_note_ip6(format, ...)
#define ns_log_info_ip6(format, ...)
#define ns_log_debg_ip6(format, ...)
#endif

#if NS_CONFIG_LOG_MAC == 1
#define ns_log_crit_mac(format, ...) ns_log_crit(NS_LOG_REGION_MAC, _NS_REGION_MAC_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_mac(format, ...) ns_log_warn(NS_LOG_REGION_MAC, _NS_REGION_MAC_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_mac(format, ...) ns_log_note(NS_LOG_REGION_MAC, _NS_REGION_MAC_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_mac(format, ...) ns_log_info(NS_LOG_REGION_MAC, _NS_REGION_MAC_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_mac(format, ...) ns_log_debg(NS_LOG_REGION_MAC, _NS_REGION_MAC_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_mac_err(error, format, ...)                                                          \
    ns_log_warn(NS_LOG_REGION_MAC, _NS_REGION_MAC_PREFIX "error %s: " format, ns_error_to_string(error), \
              ##__VA_ARGS__)
#define ns_log_mac(log_level, format, ...)                                                     \
    do                                                                                         \
    {                                                                                          \
        if (ns_logging_get_level() >= log_level)                                               \
        {                                                                                      \
            _ns_log_formatter(log_level, NS_LOG_REGION_MAC, "%s" _NS_REGION_MAC_PREFIX format, \
                            ns_log_level_to_prefix_string(log_level), ##__VA_ARGS__);          \
        }                                                                                      \
    } while (false)

#else
#define ns_log_crit_mac(format, ...)
#define ns_log_warn_mac(format, ...)
#define ns_log_note_mac(format, ...)
#define ns_log_info_mac(format, ...)
#define ns_log_debg_mac(format, ...)
#define ns_log_debg_mac_err(error, format, ...)
#define ns_log_mac(log_level, format, ...)
#endif

#if NS_CONFIG_LOG_CORE == 1
#define ns_log_crit_core(format, ...) ns_log_crit(NS_LOG_REGION_CORE, _NS_REGION_CORE_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_core(format, ...) ns_log_warn(NS_LOG_REGION_CORE, _NS_REGION_CORE_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_core(format, ...) ns_log_note(NS_LOG_REGION_CORE, _NS_REGION_CORE_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_core(format, ...) ns_log_info(NS_LOG_REGION_CORE, _NS_REGION_CORE_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_core(format, ...) ns_log_debg(NS_LOG_REGION_CORE, _NS_REGION_CORE_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_CoreErr(error, format, ...)                                                                \
    ns_log_warn(NS_LOG_REGION_CORE, _NS_REGION_CORE_PREFIX "error %s: " format, ns_error_to_string(error), \
              ##__VA_ARGS__)
#else
#define ns_log_crit_core(format, ...)
#define ns_log_warn_core(format, ...)
#define ns_log_info_core(format, ...)
#define ns_log_debg_core(format, ...)
#define ns_log_debg_CoreErr(error, format, ...)
#endif

#if NS_CONFIG_LOG_MEM == 1
#define ns_log_crit_mem(format, ...) ns_log_crit(NS_LOG_REGION_MEM, _NS_REGION_MEM_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_mem(format, ...) ns_log_warn(NS_LOG_REGION_MEM, _NS_REGION_MEM_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_mem(format, ...) ns_log_note(NS_LOG_REGION_MEM, _NS_REGION_MEM_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_mem(format, ...) ns_log_info(NS_LOG_REGION_MEM, _NS_REGION_MEM_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_mem(format, ...) ns_log_debg(NS_LOG_REGION_MEM, _NS_REGION_MEM_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_mem(format, ...)
#define ns_log_warn_mem(format, ...)
#define ns_log_note_mem(format, ...)
#define ns_log_info_mem(format, ...)
#define ns_log_debg_mem(format, ...)
#endif

#if NS_CONFIG_LOG_UTIL == 1
#define ns_log_crit_util(format, ...) ns_log_crit(NS_LOG_REGION_UTIL, _NS_REGION_UTIL_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_util(format, ...) ns_log_warn(NS_LOG_REGION_UTIL, _NS_REGION_UTIL_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_util(format, ...) ns_log_note(NS_LOG_REGION_UTIL, _NS_REGION_UTIL_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_util(format, ...) ns_log_info(NS_LOG_REGION_UTIL, _NS_REGION_UTIL_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_util_err(error, format, ...)                                                           \
    ns_log_info(NS_LOG_REGION_UTIL, _NS_REGION_CORE_PREFIX "error %s: " format, ns_error_to_string(error), \
              ##__VA_ARGS__)
#define ns_log_debg_util(format, ...) ns_log_debg(NS_LOG_REGION_UTIL, _NS_REGION_UTIL_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_util(format, ...)
#define ns_log_warn_util(format, ...)
#define ns_log_note_util(format, ...)
#define ns_log_info_util(format, ...)
#define ns_log_info_util_err(error, format, ...)
#define ns_log_debg_util(format, ...)
#endif

#if NS_CONFIG_LOG_NETDIAG == 1
#define ns_log_crit_net_diag(format, ...) \
    ns_log_crit(NS_LOG_REGION_NET_DIAG, _NS_REGION_NET_DIAG_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_net_diag(format, ...) \
    ns_log_warn(NS_LOG_REGION_NET_DIAG, _NS_REGION_NET_DIAG_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_net_diag(format, ...) \
    ns_log_note(NS_LOG_REGION_NET_DIAG, _NS_REGION_NET_DIAG_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_net_diag(format, ...) \
    ns_log_info(NS_LOG_REGION_NET_DIAG, _NS_REGION_NET_DIAG_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_net_diag(format, ...) \
    ns_log_debg(NS_LOG_REGION_NET_DIAG, _NS_REGION_NET_DIAG_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_net_diag(format, ...)
#define ns_log_warn_net_diag(format, ...)
#define ns_log_note_net_diag(format, ...)
#define ns_log_info_net_diag(format, ...)
#define ns_log_debg_net_diag(format, ...)
#endif

#if NS_ENABLE_CERT_LOG
#define ns_log__cert_mesh_cop(format, ...) \
    _ns_log_formatter(NS_LOG_LEVEL_NONE, NS_LOG_REGION_MESH_COP, format, ##__VA_ARGS__)
#else
#define ns_log__cert_mesh_cop(format, ...)
#endif

#if NS_CONFIG_LOG_CLI == 1
#define ns_log_crit_cli(format, ...) ns_log_crit(NS_LOG_REGION_CLI, _NS_REGION_CLI_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_cli(format, ...) ns_log_warn(NS_LOG_REGION_CLI, _NS_REGION_CLI_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_cli(format, ...) ns_log_note(NS_LOG_REGION_CLI, _NS_REGION_CLI_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_cli(format, ...) ns_log_info(NS_LOG_REGION_CLI, _NS_REGION_CLI_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_cli_err(error, format, ...) \
    ns_log_info(NS_LOG_REGION_CLI, "error %s: " format, ns_error_to_string(error), ##__VA_ARGS__)
#define ns_log_debg_cli(format, ...) ns_log_debg(NS_LOG_REGION_CLI, _NS_REGION_CLI_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_cli(format, ...)
#define ns_log_warn_cli(format, ...)
#define ns_log_note_cli(format, ...)
#define ns_log_info_cli(format, ...)
#define ns_log_info_cli_err(error, format, ...)
#define ns_log_debg_cli(format, ...)
#endif

#if NS_CONFIG_LOG_COAP == 1
#define ns_log_crit_coap(format, ...) ns_log_crit(NS_LOG_REGION_COAP, _NS_REGION_COAP_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_coap(format, ...) ns_log_warn(NS_LOG_REGION_COAP, _NS_REGION_COAP_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_coap(format, ...) ns_log_note(NS_LOG_REGION_COAP, _NS_REGION_COAP_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_coap(format, ...) ns_log_info(NS_LOG_REGION_COAP, _NS_REGION_COAP_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_coap_err(error, format, ...)                                                           \
    ns_log_info(NS_LOG_REGION_COAP, _NS_REGION_COAP_PREFIX "error %s: " format, ns_error_to_string(error), \
              ##__VA_ARGS__)
#define ns_log_debg_coap(format, ...) ns_log_debg(NS_LOG_REGION_COAP, _NS_REGION_COAP_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_coap(format, ...)
#define ns_log_warn_coap(format, ...)
#define ns_log_note_coap(format, ...)
#define ns_log_info_coap(format, ...)
#define ns_log_info_coap_err(error, format, ...)
#define ns_log_debg_coap(format, ...)
#endif

#if NS_CONFIG_LOG_PLATFORM == 1
#define ns_log_crit_plat(format, ...) ns_log_crit(NS_LOG_REGION_PLATFORM, _NS_REGION_PLATFORM_PREFIX format, ##__VA_ARGS__)
#define ns_log_warn_plat(format, ...) ns_log_warn(NS_LOG_REGION_PLATFORM, _NS_REGION_PLATFORM_PREFIX format, ##__VA_ARGS__)
#define ns_log_note_plat(format, ...) ns_log_note(NS_LOG_REGION_PLATFORM, _NS_REGION_PLATFORM_PREFIX format, ##__VA_ARGS__)
#define ns_log_info_plat(format, ...) ns_log_info(NS_LOG_REGION_PLATFORM, _NS_REGION_PLATFORM_PREFIX format, ##__VA_ARGS__)
#define ns_log_debg_plat(format, ...) ns_log_debg(NS_LOG_REGION_PLATFORM, _NS_REGION_PLATFORM_PREFIX format, ##__VA_ARGS__)
#else
#define ns_log_crit_plat(format, ...)
#define ns_log_warn_plat(format, ...)
#define ns_log_note_plat(format, ...)
#define ns_log_info_plat(format, ...)
#define ns_log_debg_plat(format, ...)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_CRIT
#define ns_dump_crit(region, id, buf, length) ns_dump(NS_LOG_LEVEL_CRIT, region, id, buf, length)
#else
#define ns_dump_crit(region, id, buf, length)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_WARN
#define ns_dump_warn(region, id, buf, length) ns_dump(NS_LOG_LEVEL_WARN, region, id, buf, length)
#else
#define ns_dump_warn(region, id, buf, length)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_NOTE
#define ns_dump_note(region, id, buf, length) ns_dump(NS_LOG_LEVEL_NOTE, region, id, buf, length)
#else
#define ns_dump_note(region, id, buf, length)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_INFO
#define ns_dump_info(region, id, buf, length) ns_dump(NS_LOG_LEVEL_INFO, region, id, buf, length)
#else
#define ns_dump_info(region, id, buf, length)
#endif

#if NS_CONFIG_LOG_LEVEL >= NS_LOG_LEVEL_DEBG
#define ns_dump_debg(region, id, buf, length) ns_dump(NS_LOG_LEVEL_DEBG, region, id, buf, length)
#else
#define ns_dump_debg(region, id, buf, length)
#endif

#if NS_CONFIG_LOG_NETDATA == 1
#define ns_dump_crit_net_data(id, buf, length) ns_dump_crit(NS_LOG_REGION_NET_DATA, id, buf, length)
#define ns_dump_warn_net_data(id, buf, length) ns_dump_warn(NS_LOG_REGION_NET_DATA, id, buf, length)
#define ns_dump_note_net_data(id, buf, length) ns_dump_note(NS_LOG_REGION_NET_DATA, id, buf, length)
#define ns_dump_info_net_data(id, buf, length) ns_dump_info(NS_LOG_REGION_NET_DATA, id, buf, length)
#define ns_dump_debg_net_data(id, buf, length) ns_dump_debg(NS_LOG_REGION_NET_DATA, id, buf, length)
#else
#define ns_dump_crit_net_data(id, buf, length)
#define ns_dump_warn_net_data(id, buf, length)
#define ns_dump_note_net_data(id, buf, length)
#define ns_dump_info_net_data(id, buf, length)
#define ns_dump_debg_net_data(id, buf, length)
#endif

#if NS_CONFIG_LOG_MLE == 1
#define ns_dump_crit_mle(id, buf, length) ns_dump_crit(NS_LOG_REGION_MLE, id, buf, length)
#define ns_dump_warn_mle(id, buf, length) ns_dump_warn(NS_LOG_REGION_MLE, id, buf, length)
#define ns_dump_note_mle(id, buf, length) ns_dump_note(NS_LOG_REGION_MLE, id, buf, length)
#define ns_dump_info_mle(id, buf, length) ns_dump_info(NS_LOG_REGION_MLE, id, buf, length)
#define ns_dump_debg_mle(id, buf, length) ns_dump_debg(NS_LOG_REGION_MLE, id, buf, length)
#else
#define ns_dump_crit_mle(id, buf, length)
#define ns_dump_warn_mle(id, buf, length)
#define ns_dump_note_mle(id, buf, length)
#define ns_dump_info_mle(id, buf, length)
#define ns_dump_debg_mle(id, buf, length)
#endif

#if NS_CONFIG_LOG_ARP == 1
#define ns_dump_crit_arp(id, buf, length) ns_dump_crit(NS_LOG_REGION_ARP, id, buf, length)
#define ns_dump_warn_arp(id, buf, length) ns_dump_warn(NS_LOG_REGION_ARP, id, buf, length)
#define ns_dump_note_arp(id, buf, length) ns_dump_note(NS_LOG_REGION_ARP, id, buf, length)
#define ns_dump_info_arp(id, buf, length) ns_dump_info(NS_LOG_REGION_ARP, id, buf, length)
#define ns_dump_debg_arp(id, buf, length) ns_dump_debg(NS_LOG_REGION_ARP, id, buf, length)
#else
#define ns_dump_crit_arp(id, buf, length)
#define ns_dump_warn_arp(id, buf, length)
#define ns_dump_note_arp(id, buf, length)
#define ns_dump_info_arp(id, buf, length)
#define ns_dump_debg_arp(id, buf, length)
#endif

#if NS_CONFIG_LOG_ICMP == 1
#define ns_dump_crit_icmp(id, buf, length) ns_dump_crit(NS_LOG_REGION_ICMP, id, buf, length)
#define ns_dump_warn_icmp(id, buf, length) ns_dump_warn(NS_LOG_REGION_ICMP, id, buf, length)
#define ns_dump_note_icmp(id, buf, length) ns_dump_note(NS_LOG_REGION_ICMP, id, buf, length)
#define ns_dump_info_icmp(id, buf, length) ns_dump_info(NS_LOG_REGION_ICMP, id, buf, length)
#define ns_dump_debg_icmp(id, buf, length) ns_dump_debg(NS_LOG_REGION_ICMP, id, buf, length)
#else
#define ns_dump_crit_icmp(id, buf, length)
#define ns_dump_warn_icmp(id, buf, length)
#define ns_dump_note_icmp(id, buf, length)
#define ns_dump_info_icmp(id, buf, length)
#define ns_dump_debg_icmp(id, buf, length)
#endif

#if NS_CONFIG_LOG_IP6 == 1
#define ns_dump_crit_ip6(id, buf, length) ns_dump_crit(NS_LOG_REGION_IP6, id, buf, length)
#define ns_dump_warn_ip6(id, buf, length) ns_dump_warn(NS_LOG_REGION_IP6, id, buf, length)
#define ns_dump_note_ip6(id, buf, length) ns_dump_note(NS_LOG_REGION_IP6, id, buf, length)
#define ns_dump_info_ip6(id, buf, length) ns_dump_info(NS_LOG_REGION_IP6, id, buf, length)
#define ns_dump_debg_ip6(id, buf, length) ns_dump_debg(NS_LOG_REGION_IP6, id, buf, length)
#else
#define ns_dump_crit_ip6(id, buf, length)
#define ns_dump_warn_ip6(id, buf, length)
#define ns_dump_note_ip6(id, buf, length)
#define ns_dump_info_ip6(id, buf, length)
#define ns_dump_debg_ip6(id, buf, length)
#endif

#if NS_CONFIG_LOG_MAC == 1
#define ns_dump_crit_mac(id, buf, length) ns_dump_crit(NS_LOG_REGION_MAC, id, buf, length)
#define ns_dump_warn_mac(id, buf, length) ns_dump_warn(NS_LOG_REGION_MAC, id, buf, length)
#define ns_dump_note_mac(id, buf, length) ns_dump_note(NS_LOG_REGION_MAC, id, buf, length)
#define ns_dump_info_mac(id, buf, length) ns_dump_info(NS_LOG_REGION_MAC, id, buf, length)
#define ns_dump_debg_mac(id, buf, length) ns_dump_debg(NS_LOG_REGION_MAC, id, buf, length)
#else
#define ns_dump_crit_mac(id, buf, length)
#define ns_dump_warn_mac(id, buf, length)
#define ns_dump_note_mac(id, buf, length)
#define ns_dump_info_mac(id, buf, length)
#define ns_dump_debg_mac(id, buf, length)
#endif

#if NS_CONFIG_LOG_CORE == 1
#define ns_dump_crit_core(id, buf, length) ns_dump_crit(NS_LOG_REGION_CORE, id, buf, length)
#define ns_dump_warn_core(id, buf, length) ns_dump_warn(NS_LOG_REGION_CORE, id, buf, length)
#define ns_dump_note_core(id, buf, length) ns_dump_note(NS_LOG_REGION_CORE, id, buf, length)
#define ns_dump_info_core(id, buf, length) ns_dump_info(NS_LOG_REGION_CORE, id, buf, length)
#define ns_dump_debg_core(id, buf, length) ns_dump_debg(NS_LOG_REGION_CORE, id, buf, length)
#else
#define ns_dump_crit_core(id, buf, length)
#define ns_dump_warn_core(id, buf, length)
#define ns_dump_note_core(id, buf, length)
#define ns_dump_info_core(id, buf, length)
#define ns_dump_debg_core(id, buf, length)
#endif

#if NS_CONFIG_LOG_MEM == 1
#define ns_dump_crit_mem(id, buf, length) ns_dump_crit(NS_LOG_REGION_MEM, id, buf, length)
#define ns_dump_warn_mem(id, buf, length) ns_dump_warn(NS_LOG_REGION_MEM, id, buf, length)
#define ns_dump_note_mem(id, buf, length) ns_dump_note(NS_LOG_REGION_MEM, id, buf, length)
#define ns_dump_info_mem(id, buf, length) ns_dump_info(NS_LOG_REGION_MEM, id, buf, length)
#define ns_dump_debg_mem(id, buf, length) ns_dump_debg(NS_LOG_REGION_MEM, id, buf, length)
#else
#define ns_dump_crit_mem(id, buf, length)
#define ns_dump_warn_mem(id, buf, length)
#define ns_dump_note_mem(id, buf, length)
#define ns_dump_info_mem(id, buf, length)
#define ns_dump_debg_mem(id, buf, length)
#endif

#if NS_ENABLE_CERT_LOG
#define ns_dump_cert_mesh_cop(id, buf, length) ns_dump(NS_LOG_LEVEL_NONE, NS_LOG_REGION_MESH_COP, id, buf, length)
#else
#define ns_dump_cert_mesh_cop(id, buf, length)
#endif

void
ns_dump(ns_log_level_t log_level,
        ns_log_region_t log_region,
        const char *id,
        const void *buf,
        const size_t length);

const char *
ns_log_level_to_prefix_string(ns_log_level_t log_level);

#define _ns_log_formatter(log_level, region, format, ...) \
    _ns_dynamic_log(log_level, region, format NS_CONFIG_LOG_SUFFIX, ##__VA_ARGS__)

#if NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL == 1

#define _ns_dynamic_log(log_level, region, format, ...)             \
    do                                                              \
    {                                                               \
        if (ns_logging_get_level() >= log_level)                       \
            _ns_plat_log(log_level, region, format, ##__VA_ARGS__); \
    } while (false)

#else // NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL

#define _ns_dynamic_log(log_level, region, format, ...) _ns_plat_log(log_level, region, format, ##__VA_ARGS__)

#endif // NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL

#define _ns_plat_log(log_level, region, format, ...) \
    NS_CONFIG_PLAT_LOG_FUNCTION(log_level, region, format, ##__VA_ARGS__)

#endif // NS_CORE_COMMON_LOGGING_H_
