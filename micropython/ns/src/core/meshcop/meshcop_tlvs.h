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
    MESCHOP_TLV_TYPE_EXTENDED_PANID            = NS_MESHCOP_TLV_EXTPANID,
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

#endif // NS_CORE_MESHCOP_MESHCOP_TLVS_H_
