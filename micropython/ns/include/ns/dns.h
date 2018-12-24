#ifndef NS_DNS_H_
#define NS_DNS_H_

#include "ns/ip6.h"
#include "ns/message.h"

#define NS_DNS_MAX_HOSTNAME_LENGTH 62 //maximum allowed hostname length (maximum label size - 1 for compression)

#define NS_DNS_DEFAULT_SERVER_IP "2001:4860:4860::8888"
#define NS_DNS_DEFAULT_SERVER_PORT 53 

typedef struct _ns_dns_query {
    const char *host_name;
    const ns_message_info_t *message_info;
    bool no_recursion;
} ns_dns_query_t;

typedef void (*ns_dns_response_handler_func_t)(void *context,
                                               const char *hostname,
                                               ns_ip6_addr_t *addr,
                                               uint32_t ttl,
                                               ns_error_t result);

ns_error_t
ns_dns_client_query(ns_instance_t instance,
                    const ns_dns_query_t *query,
                    ns_dns_response_handler_func_t handler,
                    void *context);

#endif // NS_DNS_H_
