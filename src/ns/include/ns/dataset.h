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

typedef uint32_t ns_channel_mask_page0;

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

#endif // NS_DATASET_H_
