#ifndef NS_INSTANCE_H_
#define NS_INSTANCE_H_

#include <stdbool.h>
#include <stdint.h>

typedef void *ns_instance_t;

enum
{
    NS_CHANGED_IP6_ADDRESS_ADDED           = 1 << 0,  // IPv6 address was added
    NS_CHANGED_IP6_ADDRESS_REMOVED         = 1 << 1,  // IPv6 address was removed
    NS_CHANGED_THREAD_ROLE                 = 1 << 2,  // Role (disabled, detached, child, router, leader) changed
    NS_CHANGED_THREAD_LL_ADDR              = 1 << 3,  // The link-local address changed
    NS_CHANGED_THREAD_ML_ADDR              = 1 << 4,  // The mesh-local address changed
    NS_CHANGED_THREAD_RLOC_ADDED           = 1 << 5,  // RLOC was added
    NS_CHANGED_THREAD_RLOC_REMOVED         = 1 << 6,  // RLOC was removed
    NS_CHANGED_THREAD_PARTITION_ID         = 1 << 7,  // Partition ID changed
    NS_CHANGED_THREAD_KEY_SEQUENCE_COUNTER = 1 << 8,  // Thread Key Sequence changed
    NS_CHANGED_THREAD_NETDATA              = 1 << 9,  // Thread Network Data changed
    NS_CHANGED_THREAD_CHILD_ADDED          = 1 << 10, // Child was added
    NS_CHANGED_THREAD_CHILD_REMOVED        = 1 << 11, // Child was removed
    NS_CHANGED_IP6_MULTICAST_SUBSRCRIBED   = 1 << 12, // Subscribed to a IPv6 multicast address
    NS_CHANGED_IP6_MULTICAST_UNSUBSRCRIBED = 1 << 13, // Unsubscribed from a IPv6 multicast address
    NS_CHANGED_COMMISSIONER_STATE          = 1 << 14, // Commissioner state changed
    NS_CHANGED_JOINER_STATE                = 1 << 15, // Joiner state changed
    NS_CHANGED_THREAD_CHANNEL              = 1 << 16, // Thread network channel changed
    NS_CHANGED_THREAD_PANID                = 1 << 17, // Thread network PAN Id changed
    NS_CHANGED_THREAD_NETWORK_NAME         = 1 << 18, // Thread network name changed
    NS_CHANGED_THREAD_EXT_PANID            = 1 << 19, // Thread network extended PAN ID changed
    NS_CHANGED_MASTER_KEY                  = 1 << 20, // Master key changed
    NS_CHANGED_PSKC                        = 1 << 21, // PSKc changed
    NS_CHANGED_SECURITY_POLICY             = 1 << 22, // Security Policy changed
    NS_CHANGED_CHANNEL_MANAGER_NEW_CHANNEL = 1 << 23, // Channel Manager new pending Thread channel changed
    NS_CHANGED_SUPPORTED_CHANNEL_MASK      = 1 << 24, // Supported channel mask changed
    NS_CHANGED_BORDER_AGENT_STATE          = 1 << 25, // Border agent state changed
    NS_CHANGED_THREAD_NETIF_STATE          = 1 << 26, // Thread network interface state changed
};

typedef uint32_t ns_changed_flags_t;

typedef void (*ns_state_changed_callback_func_t)(ns_changed_flags_t flags, void *context);

ns_instance_t
ns_instance_init(void);

ns_instance_t
ns_instance_get(void);

bool
ns_instance_is_initialized(ns_instance_t instance);

#endif // NS_INSTANCE_H_
