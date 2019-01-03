#ifndef NS_CORE_MESHCOP_MESHCOP_TLVS_H_
#define NS_CORE_MESHCOP_MESHCOP_TLVS_H_

#include "ns/commissioner.h"
#include "ns/dataset.h"
#include "core/common/crc16.h"
#include "core/common/encoding.h"
#include "core/common/message.h"
#include "core/common/tlvs.h"
#include "core/meshcop/timestamp.h"
#include "core/net/ip6_address.h"

typedef enum _meshcop_tlv_type {
    MESHCOP_TLV_TYPE_CHANNEL                   = NS_MESHCOP_TLV_CHANNEL,
    MESHCOP_TLV_TYPE_PANID                     = NS_MESHCOP_TLV_PANID,
    MESHCOP_TLV_TYPE_EXTENDED_PANID            = NS_MESHCOP_TLV_EXTPANID,
    MESHCOP_TLV_TYPE_NETWORK_NAME              = NS_MESHCOP_TLV_NETWORKNAME,
    MESHCOP_TLV_TYPE_PSKC                      = NS_MESHCOP_TLV_PSKC,
    MESHCOP_TLV_TYPE_NETWORK_MASTERKEY         = NS_MESHCOP_TLV_MASTERKEY,
    MESHCOP_TLV_TYPE_NETWORK_KEY_SEQUENCE      = NS_MESHCOP_TLV_NETWORK_KEY_SEQUENCE,
    MESHCOP_TLV_TYPE_MESH_LOCAL_PREFIX         = NS_MESHCOP_TLV_MESHLOCALPREFIX,
    MESHCOP_TLV_TYPE_STEERING_DATA             = NS_MESHCOP_TLV_STEERING_DATA,
    MESHCOP_TLV_TYPE_BORDER_AGENT_LOCATOR      = NS_MESHCOP_TLV_BORDER_AGENT_RLOC,
    MESHCOP_TLV_TYPE_COMMISSIONER_ID           = NS_MESHCOP_TLV_COMMISSIONER_ID,
    MESHCOP_TLV_TYPE_COMMISSIONER_SESSION_ID   = NS_MESHCOP_TLV_COMM_SESSION_ID,
    MESHCOP_TLV_TYPE_SECURITY_POLICY           = NS_MESHCOP_TLV_SECURITYPOLICY,
    MESHCOP_TLV_TYPE_GET                       = NS_MESHCOP_TLV_GET,
    MESHCOP_TLV_TYPE_ACTIVE_TIMESTAMP          = NS_MESHCOP_TLV_ACTIVETIMESTAMP,
    MESHCOP_TLV_TYPE_STATE                     = NS_MESHCOP_TLV_STATE,
    MESHCOP_TLV_TYPE_JOINER_DTLS_ENCAPSULATION = NS_MESHCOP_TLV_JOINER_DTLS,
    MESHCOP_TLV_TYPE_JOINER_UDP_PORT           = NS_MESHCOP_TLV_JOINER_UDP_PORT,
    MESHCOP_TLV_TYPE_JOINER_IID                = NS_MESHCOP_TLV_JOINER_IID,
    MESHCOP_TLV_TYPE_JOINER_ROUTER_LOCATOR     = NS_MESHCOP_TLV_JOINER_RLOC,
    MESHCOP_TLV_TYPE_JOINER_ROUTER_KEK         = NS_MESHCOP_TLV_JOINER_ROUTER_KEK,
    MESHCOP_TLV_TYPE_PROVISIONING_URL          = NS_MESHCOP_TLV_PROVISIONING_URL,
    MESHCOP_TLV_TYPE_VENDOR_NAME               = NS_MESHCOP_TLV_VENDOR_NAME_TLV,
    MESHCOP_TLV_TYPE_VENDOR_MODEL              = NS_MESHCOP_TLV_VENDOR_MODEL_TLV,
    MESHCOP_TLV_TYPE_VENDOR_SW_VERSION         = NS_MESHCOP_TLV_VENDOR_SW_VERSION_TLV,
    MESHCOP_TLV_TYPE_VENDOR_DATA               = NS_MESHCOP_TLV_VENDOR_DATA_TLV,
    MESHCOP_TLV_TYPE_VENDOR_STACK_VERSION      = NS_MESHCOP_TLV_VENDOR_STACK_VERSION_TLV,
    MESHCOP_TLV_TYPE_UDP_ENCAPSULATION         = NS_MESHCOP_TLV_UDP_ENCAPSULATION_TLV,
    MESHCOP_TLV_TYPE_IPV6_ADDRESS              = NS_MESHCOP_TLV_IPV6_ADDRESS_TLV,
    MESHCOP_TLV_TYPE_PENDING_TIMESTAMP         = NS_MESHCOP_TLV_PENDINGTIMESTAMP,
    MESHCOP_TLV_TYPE_DELAY_TIMER               = NS_MESHCOP_TLV_DELAYTIMER,
    MESHCOP_TLV_TYPE_CHANNEL_MASK              = NS_MESHCOP_TLV_CHANNELMASK,
    MESHCOP_TLV_TYPE_COUNT                     = NS_MESHCOP_TLV_COUNT,
    MESHCOP_TLV_TYPE_PERIOD                    = NS_MESHCOP_TLV_PERIOD,
    MESHCOP_TLV_TYPE_SCAN_DURATION             = NS_MESHCOP_TLV_SCAN_DURATION,
    MESHCOP_TLV_TYPE_ENERGY_LIST               = NS_MESHCOP_TLV_ENERGY_LIST,
    MESHCOP_TLV_TYPE_DISCOVERY_REQUEST         = NS_MESHCOP_TLV_DISCOVERYREQUEST,
    MESHCOP_TLV_TYPE_DISCOVERY_RESPONSE        = NS_MESHCOP_TLV_DISCOVERYRESPONSE,
} meshcop_tlv_type_t;

typedef tlv_t meshcop_tlv_t;

typedef extended_tlv_t meshcop_extended_tlv_t;

typedef struct _meshcop_channel_tlv {
    meshcop_tlv_t tlv;
    uint8_t channel_page;
    uint16_t channel;
} meshcop_channel_tlv_t;

typedef struct _meshcop_panid_tlv {
    meshcop_tlv_t tlv;
    uint16_t panid;
} meshcop_panid_tlv_t;

typedef struct _meshcop_extended_panid_tlv {
    meshcop_tlv_t tlv;
    ns_extended_panid_t extended_panid;
} meshcop_extended_panid_tlv_t;

typedef struct _meshcop_network_name_tlv {
    meshcop_tlv_t tlv;
    char network_name[NS_NETWORK_NAME_MAX_SIZE];
} meshcop_network_name_tlv_t;

typedef struct _meshcop_pskc_tlv {
    meshcop_tlv_t tlv;
    uint8_t pskc[16];
} meshcop_pskc_tlv_t;

typedef struct _meshcop_network_master_key_tlv {
    meshcop_tlv_t tlv;
    ns_master_key_t network_master_key;
} meshcop_network_master_key_tlv_t;

typedef struct _meshcop_network_key_sequence_tlv {
    meshcop_tlv_t tlv;
    uint32_t network_key_sequence;
} meshcop_network_key_sequence_tlv_t;

typedef struct _meshcop_mesh_local_prefix_tlv {
    meshcop_tlv_t tlv;
    ns_mesh_local_prefix_t mesh_local_prefix;
} meshcop_mesh_local_prefix_tlv_t;

typedef struct _meshcop_steering_data_tlv {
    meshcop_tlv_t tlv;
    uint8_t steering_data[NS_STEERING_DATA_MAX_LENGTH];
} meshcop_steering_data_tlv_t;

typedef struct _meshcop_border_agent_locator_tlv {
    meshcop_tlv_t tlv;
    uint16_t locator;
} meshcop_border_agent_locator_tlv_t;

enum {
    MESHCOP_COMMISSIONER_ID_TLV_MAX_LENGTH = 64,
};

typedef struct _meshcop_commissioner_id_tlv {
    meshcop_tlv_t tlv;
    char commissioner_id[MESHCOP_COMMISSIONER_ID_TLV_MAX_LENGTH];
} meshcop_commissioner_id_tlv_t;

typedef struct _meshcop_commissioner_session_id_tlv {
    meshcop_tlv_t tlv;
    uint16_t session_id;
} meshcop_commissioner_session_id_tlv_t;

enum {
    MESHCOP_SECURITY_POLICY_TLV_OBTAIN_MASTER_KEY_FLAG     = NS_SECURITY_POLICY_OBTAIN_MASTER_KEY,
    MESHCOP_SECURITY_POLICY_TLV_NATIVE_COMMISSIONING_FLAG  = NS_SECURITY_POLICY_NATIVE_COMISSIONING,
    MESHCOP_SECURITY_POLICY_TLV_ROUTERS_FLAG               = NS_SECURITY_POLICY_ROUTERS,
    MESHCOP_SECURITY_POLICY_TLV_EXTERNAL_COMMISSIONER_FLAG = NS_SECURITY_POLICY_EXTERNAL_COMMISSIONER,
    MESHCOP_SECURITY_POLICY_TLV_BEACONS_FLAG               = NS_SECURITY_POLICY_BEACONS,
};

typedef struct _meshcop_security_policy_tlv {
    meshcop_tlv_t tlv;
    uint16_t rotation_time;
    uint8_t flags;
} meshcop_security_policy_tlv_t;

typedef struct _meshcop_active_timestamp_tlv {
    meshcop_tlv_t tlv;
    meshcop_timestamp_t timestamp;
} meshcop_active_timestamp_tlv_t;

typedef enum _meshcop_state_tlv_state {
    MESHCOP_STATE_TLV_REJECT  = -1,
    MESHCOP_STATE_TLV_PENDING = 0,
    MESHCOP_STATE_TLV_ACCEPT  = 1,
} meshcop_state_tlv_state_t;

typedef struct _meshcop_state_tlv {
    meshcop_tlv_t tlv;
    uint8_t state;
} meshcop_state_tlv_t;

typedef struct _meshcop_joiner_udp_port_tlv {
    meshcop_tlv_t tlv;
    uint16_t udp_port;
} meshcop_joiner_udp_port_tlv_t;

typedef struct _meshcop_joiner_iid_tlv {
    meshcop_tlv_t tlv;
    uint8_t iid[8];
} meshcop_joiner_iid_tlv_t;

typedef struct _meshcop_joiner_router_locator_tlv {
    meshcop_tlv_t tlv;
    uint16_t locator;
} meshcop_joiner_router_locator_tlv_t;

typedef struct _meshcop_joiner_router_kek_tlv {
    meshcop_tlv_t tlv;
    uint8_t kek[16];
} meshcop_joiner_router_kek_tlv_t;

typedef struct _meshcop_pending_timestamp_tlv {
    meshcop_tlv_t tlv;
    meshcop_timestamp_t timestamp;
} meshcop_pending_timestamp_tlv_t;

enum {
    MESHCOP_DELAY_TIMER_TLV_MAX_DELAY_TIMER     = 259200,
    MESHCOP_DELAY_TIMER_TLV_DELAY_TIMER_MINIMAL = NS_CONFIG_MESHCOP_PENDING_DATASET_MINIMUM_DELAY,
    MESHCOP_DELAY_TIMER_TLV_DELAY_TIMER_DEFAULT = NS_CONFIG_MESHCOP_PENDING_DATASET_DEFAULT_DELAY,
};

typedef struct _meshcop_delay_timer_tlv {
    meshcop_tlv_t tlv;
    uint32_t delay_timer;
} meshcop_delay_timer_tlv_t;

typedef struct _meshcop_channel_mask_entry_base_tlv {
    uint8_t channel_page;
    uint8_t mask_length;
} meshcop_channel_mask_entry_base_tlv_t;

// --- meshcop tlv functions
meshcop_tlv_type_t
meshcop_tlv_get_type(meshcop_tlv_t *meshcop_tlv);

void
meshcop_tlv_set_type(meshcop_tlv_t *meshcop_tlv, meshcop_tlv_type_t type);

meshcop_tlv_t *
meshcop_tlv_get_next(meshcop_tlv_t *meshcop_tlv);

ns_error_t
meshcop_tlv_get_tlv(const message_t message,
                    meshcop_tlv_type_t type,
                    uint16_t max_length,
                    meshcop_tlv_t *tlv);

ns_error_t
meshcop_tlv_get_value_offset(const message_t message,
                             meshcop_tlv_type_t type,
                             uint16_t *offset,
                             uint16_t *length);

bool
meshcop_tlv_is_valid(const meshcop_tlv_t *tlv);

// --- meshcop extended tlv functions
meshcop_tlv_type_t
meshcop_extended_tlv_get_type(meshcop_extended_tlv_t *meshcop_extended_tlv);

void
meshcop_extended_tlv_set_type(meshcop_extended_tlv_t *meshcop_extended_tlv, meshcop_tlv_type_t type);

// --- meshcop channel tlv fuctions
void
meshcop_channel_tlv_init(meshcop_channel_tlv_t *meshcop_channel_tlv);

bool
meshcop_channel_tlv_is_valid(meshcop_channel_tlv_t *meshcop_channel_tlv);

uint8_t
meshcop_channel_tlv_get_channel_page(meshcop_channel_tlv_t *meshcop_channel_tlv);

void
meshcop_channel_tlv_set_channel_page(meshcop_channel_tlv_t *meshcop_channel_tlv, uint8_t channel_page);

uint16_t
meshcop_channel_tlv_get_channel(meshcop_channel_tlv_t *meshcop_channel_tlv);

void
meshcop_channel_tlv_set_channel(meshcop_channel_tlv_t *meshcop_channel_tlv, uint16_t channel);

// --- panid tlv functions
void
meshcop_panid_tlv_init(meshcop_panid_tlv_t *meshcop_panid_tlv);

bool
meshcop_panid_tlv_is_valid(meshcop_panid_tlv_t *meshcop_panid_tlv);

uint16_t
meshcop_panid_tlv_get_panid(meshcop_panid_tlv_t *meshcop_panid_tlv);

void
meshcop_panid_tlv_set_panid(meshcop_panid_tlv_t *meshcop_panid_tlv, uint16_t panid);

// --- extended panid tlv functions
void
meshcop_extended_panid_tlv_init(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv);

bool
meshcop_extended_panid_tlv_is_valid(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv);

const ns_extended_panid_t
meshcop_extended_panid_tlv_get_extended_panid(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv);

void
meshcop_extended_panid_tlv_set_extended_panid(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv,
                                              const ns_extended_panid_t extended_panid);

// --- network name tlv functions
void
meshcop_network_name_tlv_init(meshcop_network_name_tlv_t *meshcop_network_name_tlv);

bool
meshcop_network_name_tlv_is_valid(meshcop_network_name_tlv_t *meshcop_network_name_tlv);

const char *
meshcop_network_name_tlv_get_network_name(meshcop_network_name_tlv_t *meshcop_network_name_tlv);

void
meshcop_network_name_tlv_set_network_name(meshcop_network_name_tlv_t *meshcop_network_name_tlv,
                                          const char *network_name);

// --- pskc tlv functions
void
meshcop_pskc_tlv_init(meshcop_pskc_tlv_t *meshcop_pskc_tlv);

bool
meshcop_pskc_tlv_is_valid(meshcop_pskc_tlv_t *meshcop_pskc_tlv);

const uint8_t *
meshcop_pskc_tlv_get_pskc(meshcop_pskc_tlv_t *meshcop_pskc_tlv);

void
meshcop_pskc_tlv_set_pskc(meshcop_pskc_tlv_t *meshcop_pskc_tlv, const uint8_t *pskc);

// --- network master key tlv functions
void
meshcop_network_master_key_tlv_init(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv);

bool
meshcop_network_master_key_tlv_is_valid(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv);

const ns_master_key_t
meshcop_network_master_key_tlv_get_network_master_key(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv);

void
meshcop_network_master_key_tlv_set_network_master_key(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv, const ns_master_key_t network_master_key);

// --- network sequence tlv functions
void
meshcop_network_key_sequence_tlv_init(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv);

bool
meshcop_network_key_sequence_tlv_is_valid(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv);

uint32_t
meshcop_network_key_sequence_tlv_get_network_key_sequence(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv);

void
meshcop_network_key_sequence_tlv_set_network_key_sequence(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv, uint32_t network_key_sequence);

// --- mesh local prefix tlv functions
void
meshcop_mesh_local_prefix_tlv_init(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv);

bool
meshcop_mesh_local_prefix_tlv_is_valid(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv);

const ns_mesh_local_prefix_t
meshcop_mesh_local_prefix_tlv_get_mesh_local_prefix(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv);

void
meshcop_mesh_local_prefix_tlv_set_mesh_local_prefix(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv, const ns_mesh_local_prefix_t mesh_local_prefix);

// --- steering data tlv
void
meshcop_steering_data_tlv_init(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv);

bool
meshcop_steering_data_tlv_is_valid(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv);

void
meshcop_steering_data_tlv_clear(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv);

void
meshcop_steering_data_tlv_set(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv);

bool
meshcop_steering_data_tlv_does_allow_any(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv);

uint8_t
meshcop_steering_data_tlv_get_num_bits(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv);

bool
meshcop_steering_data_tlv_get_bit(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv, uint8_t bit);

void
meshcop_steering_data_tlv_clear_bit(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv, uint8_t bit);

void
meshcop_steering_data_tlv_set_bit(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv, uint8_t bit);

bool
meshcop_steering_data_tlv_is_cleared(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv);

void
meshcop_steering_data_tlv_compute_bloom_filter(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv,
                                               const ns_ext_addr_t joiner_id);

// --- border agent locator tlv
void
meshcop_border_agent_locator_tlv_init(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv);

bool
meshcop_border_agent_locator_tlv_is_valid(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv);

uint16_t
meshcop_border_agent_locator_tlv_get_border_agent_locator(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv);

void
meshcop_border_agent_locator_tlv_set_border_agent_locator(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv, uint16_t locator);

// --- commissioner id tlv
void
meshcop_commissioner_id_tlv_init(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv);

bool
meshcop_commissioner_id_tlv_is_valid(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv);

const char *
meshcop_commissioner_id_tlv_get_commissioner_id(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv);

void
meshcop_commissioner_id_tlv_set_commissioner_id(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv,
                                                const char *commissioner_id);

// --- commissioner session id tlv
void
meshcop_commissioner_session_id_tlv_init(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv);

bool
meshcop_commissioner_session_id_tlv_is_valid(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv);

uint16_t
meshcop_commissioner_session_id_tlv_get_commissioner_session_id(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv);

void
meshcop_commissioner_session_id_tlv_set_commissioner_session_id(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv, uint16_t session_id);

// --- security policy tlv
void
meshcop_security_policy_tlv_init(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv);

bool
meshcop_security_policy_tlv_is_valid(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv);

uint16_t
meshcop_security_policy_tlv_get_rotation_time(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv);

void
meshcop_security_policy_tlv_set_rotation_time(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv, uint16_t rotation_time);

uint8_t
meshcop_security_policy_tlv_get_flags(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv);

void
meshcop_security_policy_tlv_set_flags(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv, uint8_t flags);

// --- active timestamp tlv
void
meshcop_active_timestamp_tlv_init(meshcop_active_timestamp_tlv_t *meshcop_active_timestamp_tlv);

bool
meshcop_active_timestamp_tlv_is_valid(meshcop_active_timestamp_tlv_t *meshcop_active_timestamp_tlv);

// --- state tlv
void
meshcop_state_tlv_init(meshcop_state_tlv_t *meshcop_state_tlv);

bool
meshcop_state_tlv_is_valid(meshcop_state_tlv_t *meshcop_state_tlv);

meshcop_state_tlv_state_t
meshcop_state_tlv_get_state(meshcop_state_tlv_t *meshcop_state_tlv);

void
meshcop_state_tlv_set_state(meshcop_state_tlv_t *meshcop_state_tlv, meshcop_state_tlv_state_t state);

// --- joiner udp port tlv functions
void
meshcop_joiner_udp_port_tlv_init(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv);

bool
meshcop_joiner_udp_port_tlv_is_valid(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv);

uint16_t
meshcop_joiner_udp_port_tlv_get_udp_port(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv);

void
meshcop_joiner_udp_port_tlv_set_udp_port(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv, uint16_t udp_port);

// --- joiner iid tlv functions
void
meshcop_joiner_iid_tlv_init(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv);

bool
meshcop_joiner_iid_tlv_is_valid(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv);

const uint8_t *
meshcop_joiner_iid_tlv_get_iid(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv);

void
meshcop_joiner_iid_tlv_set_iid(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv, const uint8_t *iid);

// --- joiner router locator tlv functions
void
meshcop_joiner_router_locator_tlv_init(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv);

bool
meshcop_joiner_router_locator_tlv_is_valid(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv);

uint16_t
meshcop_joiner_router_locator_tlv_get_joiner_router_locator(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv);

void
meshcop_joiner_router_locator_tlv_set_joiner_router_locator(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv, uint16_t locator);

// --- joiner router kek tlv functions
void
meshcop_joiner_router_kek_tlv_init(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv);

bool
meshcop_joiner_router_kek_tlv_is_valid(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv);

const uint8_t *
meshcop_joiner_router_kek_tlv_get_kek(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv);

void
meshcop_joiner_router_kek_tlv_set_kek(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv, const uint8_t *kek);

// --- pending timestamp tlv functions
void
meshcop_pending_timestamp_tlv_init(meshcop_pending_timestamp_tlv_t *meshcop_pending_timestamp_tlv);

bool
meshcop_pending_timestamp_tlv_is_valid(meshcop_pending_timestamp_tlv_t *meshcop_pending_timestamp_tlv);

// --- delay timer tlv functions
void
meshcop_delay_timer_tlv_init(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv);

bool
meshcop_delay_timer_tlv_is_valid(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv);

uint32_t
meshcop_delay_timer_tlv_get_delay_timer(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv);

void
meshcop_delay_timer_tlv_set_delay_timer(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv, uint32_t delay_timer);

// --- channel mask entry base tlv functions
uint8_t
meshcop_channel_mask_entry_base_tlv_get_channel_page(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv);

void
meshcop_channel_mask_entry_base_tlv_set_channel_page(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv,
                                                     uint8_t channel_page);

uint8_t
meshcop_channel_mask_entry_base_tlv_get_mask_length(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv);

void
meshcop_channel_mask_entry_base_tlv_set_mask_length(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv, uint8_t mask_length);

uint16_t
meshcop_channel_mask_entry_base_tlv_get_size(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv);

void
meshcop_channel_mask_entry_base_tlv_clear_channel(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv, uint8_t channel);

bool
meshcop_channel_mask_entry_base_tlv_is_channel_set(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv, uint8_t channel);

const meshcop_channel_mask_entry_base_tlv_t *
meshcop_channel_mask_entry_base_tlv_get_next(meshcop_channel_mask_entry_base_tlv_t *meshcop_channel_mask_entry_base_tlv, const meshcop_tlv_t *channel_mask_base_tlv);

#endif // NS_CORE_MESHCOP_MESHCOP_TLVS_H_
