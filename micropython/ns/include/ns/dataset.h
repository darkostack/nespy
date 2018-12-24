#ifndef NS_DATASET_H_
#define NS_DATASET_H_

#include "ns/instance.h"
#include "ns/platform/radio.h"

#define NS_MASTER_KEY_SIZE 16 // size of the Thread Master Key (bytes)

struct _ns_master_key {
    uint8_t m8[NS_MASTER_KEY_SIZE];
};

typedef struct _ns_master_key ns_master_key_t;

#define NS_NETWORK_NAME_MAX_SIZE 16 // maximum size of the Thread Network Name field (bytes)

typedef struct _ns_network_name {
    char m8[NS_NETWORK_NAME_MAX_SIZE + 1];
} ns_network_name_t;

#define NS_EXT_PANID_SIZE 8 // size of the Thread PAN ID (bytes)

struct _ns_extended_panid {
    uint8_t m8[NS_EXT_PANID_SIZE];
};

typedef struct _ns_extended_panid ns_extended_panid_t;

#define NS_MESH_LOCAL_PREFIX_SIZE 8 // size of the Mesh Local Prefix (bytes)

struct _ns_mesh_local_prefix {
    uint8_t m8[NS_MESH_LOCAL_PREFIX_SIZE];
};

typedef struct _ns_mesh_local_prefix ns_mesh_local_prefix_t;

#define NS_PSKC_MAX_SIZE 16 // maximum size of the PSKc (bytes)

typedef struct _ns_pskc {
    uint8_t m8[NS_PSKC_MAX_SIZE];
} ns_pskc_t;

typedef struct _ns_security_policy {
    uint16_t rotation_time; // the value for thrKeyRotation in units of hours
    uint16_t flags;         // flags as defined in Thread 1.1 Section 8.10.1.15
} ns_security_policy_t;

// This enumeration defines the security policy TLV flags
enum {
    NS_SECURITY_POLICY_OBTAIN_MASTER_KEY     = 1 << 7,
    NS_SECURITY_POLICY_NATIVE_COMISSIONING   = 1 << 6,
    NS_SECURITY_POLICY_ROUTERS               = 1 << 5,
    NS_SECURITY_POLICY_EXTERNAL_COMMISSIONER = 1 << 4,
    NS_SECURITY_POLICY_BEACONS               = 1 << 3,
};

typedef uint32_t ns_channel_mask_page0_t;

#define NS_CHANNEL_11_MASK (1 << 11) // Channel 11
#define NS_CHANNEL_12_MASK (1 << 12) // Channel 12
#define NS_CHANNEL_13_MASK (1 << 13) // Channel 13
#define NS_CHANNEL_14_MASK (1 << 14) // Channel 14
#define NS_CHANNEL_15_MASK (1 << 15) // Channel 15
#define NS_CHANNEL_16_MASK (1 << 16) // Channel 16
#define NS_CHANNEL_17_MASK (1 << 17) // Channel 17
#define NS_CHANNEL_18_MASK (1 << 18) // Channel 18
#define NS_CHANNEL_19_MASK (1 << 19) // Channel 19
#define NS_CHANNEL_20_MASK (1 << 20) // Channel 20
#define NS_CHANNEL_21_MASK (1 << 21) // Channel 21
#define NS_CHANNEL_22_MASK (1 << 22) // Channel 22
#define NS_CHANNEL_23_MASK (1 << 23) // Channel 23
#define NS_CHANNEL_24_MASK (1 << 24) // Channel 24
#define NS_CHANNEL_25_MASK (1 << 25) // Channel 25
#define NS_CHANNEL_26_MASK (1 << 26) // Channel 26

#define NS_CHANNEL_ALL 0xffffffff // All channels

typedef struct _ns_operational_dataset_components {
    bool is_active_timestamp_present : 1;
    bool is_pending_timestamp_present : 1;
    bool is_master_key_present : 1;
    bool is_network_name_present : 1;
    bool is_extended_panid_present : 1;
    bool is_mesh_local_prefix_present : 1;
    bool is_delay_present : 1;
    bool is_panid_present : 1;
    bool is_channel_present : 1;
    bool is_pskc_present : 1;
    bool is_security_policy_present : 1;
    bool is_channel_mask_page0_present : 1;
} ns_operational_dataset_components_t;

typedef struct _ns_operational_dataset {
    uint64_t active_timestamp;
    uint64_t pending_timestamp;
    ns_master_key_t master_key;
    ns_network_name_t network_name;
    ns_extended_panid_t extended_panid;
    ns_mesh_local_prefix_t mesh_local_prefix;
    uint32_t delay;
    ns_panid_t panid;
    uint16_t channel;
    ns_pskc_t pskc;
    ns_security_policy_t security_policy;
    ns_channel_mask_page0_t channel_mask_page0;
    ns_operational_dataset_components_t components;
} ns_operational_dataset_t;

typedef enum _ns_meshcop_tlv_type {
    NS_MESHCOP_TLV_CHANNEL                  = 0,   // meshcop Channel TLV
    NS_MESHCOP_TLV_PANID                    = 1,   // meshcop Pan Id TLV
    NS_MESHCOP_TLV_EXTPANID                 = 2,   // meshcop Extended Pan Id TLV
    NS_MESHCOP_TLV_NETWORKNAME              = 3,   // meshcop Network Name TLV
    NS_MESHCOP_TLV_PSKC                     = 4,   // meshcop PSKc TLV
    NS_MESHCOP_TLV_MASTERKEY                = 5,   // meshcop Network Master Key TLV
    NS_MESHCOP_TLV_NETWORK_KEY_SEQUENCE     = 6,   // meshcop Network Key Sequence TLV
    NS_MESHCOP_TLV_MESHLOCALPREFIX          = 7,   // meshcop Mesh Local Prefix TLV
    NS_MESHCOP_TLV_STEERING_DATA            = 8,   // meshcop Steering Data TLV
    NS_MESHCOP_TLV_BORDER_AGENT_RLOC        = 9,   // meshcop Border Agent Locator TLV
    NS_MESHCOP_TLV_COMMISSIONER_ID          = 10,  // meshcop Commissioner ID TLV
    NS_MESHCOP_TLV_COMM_SESSION_ID          = 11,  // meshcop Commissioner Session ID TLV
    NS_MESHCOP_TLV_SECURITYPOLICY           = 12,  // meshcop Security Policy TLV
    NS_MESHCOP_TLV_GET                      = 13,  // meshcop Get TLV
    NS_MESHCOP_TLV_ACTIVETIMESTAMP          = 14,  // meshcop Active Timestamp TLV
    NS_MESHCOP_TLV_STATE                    = 16,  // meshcop State TLV
    NS_MESHCOP_TLV_JOINER_DTLS              = 17,  // meshcop Joiner DTLS Encapsulation TLV
    NS_MESHCOP_TLV_JOINER_UDP_PORT          = 18,  // meshcop Joiner UDP Port TLV
    NS_MESHCOP_TLV_JOINER_IID               = 19,  // meshcop Joiner IID TLV
    NS_MESHCOP_TLV_JOINER_RLOC              = 20,  // meshcop Joiner Router Locator TLV
    NS_MESHCOP_TLV_JOINER_ROUTER_KEK        = 21,  // meshcop Joiner Router KEK TLV
    NS_MESHCOP_TLV_PROVISIONING_URL         = 32,  // meshcop Provisioning URL TLV
    NS_MESHCOP_TLV_VENDOR_NAME_TLV          = 33,  // meshcop Vendor Name TLV
    NS_MESHCOP_TLV_VENDOR_MODEL_TLV         = 34,  // meshcop Vendor Model TLV
    NS_MESHCOP_TLV_VENDOR_SW_VERSION_TLV    = 35,  // meshcop Vendor SW Version TLV
    NS_MESHCOP_TLV_VENDOR_DATA_TLV          = 36,  // meshcop Vendor Data TLV
    NS_MESHCOP_TLV_VENDOR_STACK_VERSION_TLV = 37,  // meshcop Vendor Stack Version TLV
    NS_MESHCOP_TLV_UDP_ENCAPSULATION_TLV    = 48,  // meshcop UDP encapsulation TLV
    NS_MESHCOP_TLV_IPV6_ADDRESS_TLV         = 49,  // meshcop IPv6 address TLV
    NS_MESHCOP_TLV_PENDINGTIMESTAMP         = 51,  // meshcop Pending Timestamp TLV
    NS_MESHCOP_TLV_DELAYTIMER               = 52,  // meshcop Delay Timer TLV
    NS_MESHCOP_TLV_CHANNELMASK              = 53,  // meshcop Channel Mask TLV
    NS_MESHCOP_TLV_COUNT                    = 54,  // meshcop Count TLV
    NS_MESHCOP_TLV_PERIOD                   = 55,  // meshcop Period TLV
    NS_MESHCOP_TLV_SCAN_DURATION            = 56,  // meshcop Scan Duration TLV
    NS_MESHCOP_TLV_ENERGY_LIST              = 57,  // meshcop Energy List TLV
    NS_MESHCOP_TLV_DISCOVERYREQUEST         = 128, // meshcop Discovery Request TLV
    NS_MESHCOP_TLV_DISCOVERYRESPONSE        = 129, // meshcop Discovery Response TLV
} ns_meshcop_tlv_type_t;

bool
ns_dataset_is_commisioned(ns_instance_t instance);

ns_error_t
ns_dataset_get_active(ns_instance_t instance, ns_operational_dataset_t *dataset);

ns_error_t
ns_dataset_get_pending(ns_instance_t instance, ns_operational_dataset_t *dataset);

#endif // NS_DATASET_H_
