#include "core/meshcop/meshcop_tlvs.h"
#include "core/common/code_utils.h"

// --- meshcop tlv functions
meshcop_tlv_type_t
meshcop_tlv_get_type(meshcop_tlv_t *meshcop_tlv)
{
    return (meshcop_tlv_type_t)tlv_get_type((tlv_t *)meshcop_tlv);
}

void
meshcop_tlv_set_type(meshcop_tlv_t *meshcop_tlv, meshcop_tlv_type_t type)
{
    tlv_set_type((tlv_t *)meshcop_tlv, (uint8_t)type);
}

meshcop_tlv_t *
meshcop_tlv_get_next(meshcop_tlv_t *meshcop_tlv)
{
    return (meshcop_tlv_t *)tlv_get_next((tlv_t *)meshcop_tlv);
}

ns_error_t
meshcop_tlv_get_tlv(const message_t message,
                    meshcop_tlv_type_t type,
                    uint16_t max_length,
                    meshcop_tlv_t *tlv)
{
    return tlv_get(message, (uint8_t)type, max_length, (tlv_t *)tlv);
}

ns_error_t
meshcop_tlv_get_value_offset(const message_t message,
                             meshcop_tlv_type_t type,
                             uint16_t *offset,
                             uint16_t *length)
{
    return tlv_get_value_offset(message, (uint8_t)type, offset, length);
}

bool
meshcop_tlv_is_valid(const meshcop_tlv_t *tlv)
{
    // TODO:
    return false;
}

// --- meshcop extended tlv functions
meshcop_tlv_type_t
meshcop_extended_tlv_get_type(meshcop_extended_tlv_t *meshcop_extended_tlv)
{
    return (meshcop_tlv_type_t)tlv_get_type((tlv_t *)&meshcop_extended_tlv->tlv);
}

void
meshcop_extended_tlv_set_type(meshcop_extended_tlv_t *meshcop_extended_tlv, meshcop_tlv_type_t type)
{
    tlv_set_type((tlv_t *)&meshcop_extended_tlv->tlv, (uint8_t)type);
}

// --- meshcop channel tlv fuctions
void
meshcop_channel_tlv_init(meshcop_channel_tlv_t *meshcop_channel_tlv)
{
    meshcop_tlv_set_type(&meshcop_channel_tlv->tlv, MESHCOP_TLV_TYPE_CHANNEL);
    tlv_set_length(&meshcop_channel_tlv->tlv, sizeof(*meshcop_channel_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_channel_tlv_is_valid(meshcop_channel_tlv_t *meshcop_channel_tlv)
{
    return tlv_get_length(&meshcop_channel_tlv->tlv) == sizeof(*meshcop_channel_tlv) - sizeof(meshcop_tlv_t);
}

uint8_t
meshcop_channel_tlv_get_channel_page(meshcop_channel_tlv_t *meshcop_channel_tlv)
{
    return meshcop_channel_tlv->channel_page;
}

void
meshcop_channel_tlv_set_channel_page(meshcop_channel_tlv_t *meshcop_channel_tlv, uint8_t channel_page)
{
    meshcop_channel_tlv->channel_page = channel_page;
}

uint16_t
meshcop_channel_tlv_get_channel(meshcop_channel_tlv_t *meshcop_channel_tlv)
{
    return encoding_big_endian_swap16(meshcop_channel_tlv->channel);
}

void
meshcop_channel_tlv_set_channel(meshcop_channel_tlv_t *meshcop_channel_tlv, uint16_t channel)
{
    meshcop_channel_tlv->channel = encoding_big_endian_swap16(channel);
}

// --- panid tlv functions
void
meshcop_panid_tlv_init(meshcop_panid_tlv_t *meshcop_panid_tlv)
{
    meshcop_tlv_set_type(&meshcop_panid_tlv->tlv, MESHCOP_TLV_TYPE_PANID);
    tlv_set_length(&meshcop_panid_tlv->tlv, sizeof(*meshcop_panid_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_panid_tlv_is_valid(meshcop_panid_tlv_t *meshcop_panid_tlv)
{
    return tlv_get_length(&meshcop_panid_tlv->tlv) == sizeof(*meshcop_panid_tlv) - sizeof(meshcop_tlv_t);
}

uint16_t
meshcop_panid_tlv_get_panid(meshcop_panid_tlv_t *meshcop_panid_tlv)
{
    return encoding_big_endian_swap16(meshcop_panid_tlv->panid);
}

void
meshcop_panid_tlv_set_panid(meshcop_panid_tlv_t *meshcop_panid_tlv, uint16_t panid)
{
    meshcop_panid_tlv->panid = encoding_big_endian_swap16(panid);
}

// --- extended panid tlv functions
void
meshcop_extended_panid_tlv_init(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv)
{
    meshcop_tlv_set_type(&meshcop_extended_panid_tlv->tlv, MESHCOP_TLV_TYPE_EXTENDED_PANID);
    tlv_set_length(&meshcop_extended_panid_tlv->tlv, sizeof(*meshcop_extended_panid_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_extended_panid_tlv_is_valid(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv)
{
    return tlv_get_length(&meshcop_extended_panid_tlv->tlv) == sizeof(*meshcop_extended_panid_tlv) - sizeof(meshcop_tlv_t);
}

const ns_extended_panid_t
meshcop_extended_panid_tlv_get_extended_panid(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv)
{
    return meshcop_extended_panid_tlv->extended_panid;
}

void
meshcop_extended_panid_tlv_set_extended_panid(meshcop_extended_panid_tlv_t *meshcop_extended_panid_tlv,
                                              const ns_extended_panid_t extended_panid)
{
    meshcop_extended_panid_tlv->extended_panid = extended_panid;
}

// --- network name tlv functions
void
meshcop_network_name_tlv_init(meshcop_network_name_tlv_t *meshcop_network_name_tlv)
{
    meshcop_tlv_set_type(&meshcop_network_name_tlv->tlv, MESHCOP_TLV_TYPE_NETWORK_NAME);
    tlv_set_length(&meshcop_network_name_tlv->tlv, sizeof(*meshcop_network_name_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_network_name_tlv_is_valid(meshcop_network_name_tlv_t *meshcop_network_name_tlv)
{
    return tlv_get_length(&meshcop_network_name_tlv->tlv) <= sizeof(*meshcop_network_name_tlv) - sizeof(meshcop_tlv_t);
}

const char *
meshcop_network_name_tlv_get_network_name(meshcop_network_name_tlv_t *meshcop_network_name_tlv)
{
    return meshcop_network_name_tlv->network_name;
}

void
meshcop_network_name_tlv_set_network_name(meshcop_network_name_tlv_t *meshcop_network_name_tlv,
                                          const char *network_name)
{
    size_t length = strnlen(network_name, sizeof(meshcop_network_name_tlv->network_name));
    memcpy(meshcop_network_name_tlv->network_name, network_name, length);
    tlv_set_length(&meshcop_network_name_tlv->tlv, (uint8_t)length);
}

// --- pskc tlv functions
void
meshcop_pskc_tlv_init(meshcop_pskc_tlv_t *meshcop_pskc_tlv)
{
    meshcop_tlv_set_type(&meshcop_pskc_tlv->tlv, MESHCOP_TLV_TYPE_PSKC);
    tlv_set_length(&meshcop_pskc_tlv->tlv, sizeof(*meshcop_pskc_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_pskc_tlv_is_valid(meshcop_pskc_tlv_t *meshcop_pskc_tlv)
{
    return tlv_get_length(&meshcop_pskc_tlv->tlv) == sizeof(*meshcop_pskc_tlv) - sizeof(meshcop_tlv_t);
}

const uint8_t *
meshcop_pskc_tlv_get_pskc(meshcop_pskc_tlv_t *meshcop_pskc_tlv)
{
    return meshcop_pskc_tlv->pskc;
}

void
meshcop_pskc_tlv_set_pskc(meshcop_pskc_tlv_t *meshcop_pskc_tlv, const uint8_t *pskc)
{
    memcpy(meshcop_pskc_tlv->pskc, pskc, sizeof(meshcop_pskc_tlv->pskc));
}

// --- network master key tlv functions
void
meshcop_network_master_key_tlv_init(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv)
{
    meshcop_tlv_set_type(&meshcop_network_master_key_tlv->tlv, MESHCOP_TLV_TYPE_NETWORK_MASTERKEY);
    tlv_set_length(&meshcop_network_master_key_tlv->tlv, sizeof(*meshcop_network_master_key_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_network_master_key_tlv_is_valid(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv)
{
    return tlv_get_length(&meshcop_network_master_key_tlv->tlv) == sizeof(*meshcop_network_master_key_tlv) - sizeof(meshcop_tlv_t);
}

const ns_master_key_t
meshcop_network_master_key_tlv_get_network_master_key(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv)
{
    return meshcop_network_master_key_tlv->network_master_key;
}

void
meshcop_network_master_key_tlv_set_network_master_key(meshcop_network_master_key_tlv_t *meshcop_network_master_key_tlv, const ns_master_key_t network_master_key)
{
    meshcop_network_master_key_tlv->network_master_key = network_master_key;
}

// --- network key sequence tlv functions
void
meshcop_network_key_sequence_tlv_init(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv)
{
    meshcop_tlv_set_type(&meshcop_network_key_sequence_tlv->tlv, MESHCOP_TLV_TYPE_NETWORK_KEY_SEQUENCE);
    tlv_set_length(&meshcop_network_key_sequence_tlv->tlv, sizeof(*meshcop_network_key_sequence_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_network_key_sequence_tlv_is_valid(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv)
{
    return tlv_get_length(&meshcop_network_key_sequence_tlv->tlv) == sizeof(*meshcop_network_key_sequence_tlv) - sizeof(meshcop_tlv_t);
}

uint32_t
meshcop_network_key_sequence_tlv_get_network_key_sequence(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv)
{
    return encoding_big_endian_swap32(meshcop_network_key_sequence_tlv->network_key_sequence);
}

void
meshcop_network_key_sequence_tlv_set_network_key_sequence(meshcop_network_key_sequence_tlv_t *meshcop_network_key_sequence_tlv, uint32_t network_key_sequence)
{
    meshcop_network_key_sequence_tlv->network_key_sequence = encoding_big_endian_swap32(network_key_sequence);
}

// --- mesh local prefix tlv functions
void
meshcop_mesh_local_prefix_tlv_init(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv)
{
    meshcop_tlv_set_type(&meshcop_mesh_local_prefix_tlv->tlv, MESHCOP_TLV_TYPE_MESH_LOCAL_PREFIX);
    tlv_set_length(&meshcop_mesh_local_prefix_tlv->tlv, sizeof(*meshcop_mesh_local_prefix_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_mesh_local_prefix_tlv_is_valid(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv)
{
    return tlv_get_length(&meshcop_mesh_local_prefix_tlv->tlv) == sizeof(*meshcop_mesh_local_prefix_tlv) - sizeof(meshcop_tlv_t);
}

const ns_mesh_local_prefix_t
meshcop_mesh_local_prefix_tlv_get_mesh_local_prefix(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv)
{
    return meshcop_mesh_local_prefix_tlv->mesh_local_prefix;
}

void
meshcop_mesh_local_prefix_tlv_set_mesh_local_prefix(meshcop_mesh_local_prefix_tlv_t *meshcop_mesh_local_prefix_tlv, const ns_mesh_local_prefix_t mesh_local_prefix)
{
    meshcop_mesh_local_prefix_tlv->mesh_local_prefix = mesh_local_prefix;
}

// --- steering data tlv
void
meshcop_steering_data_tlv_init(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv)
{
    meshcop_tlv_set_type(&meshcop_steering_data_tlv->tlv, MESHCOP_TLV_TYPE_STEERING_DATA);
    tlv_set_length(&meshcop_steering_data_tlv->tlv, sizeof(*meshcop_steering_data_tlv) - sizeof(meshcop_tlv_t));
    meshcop_steering_data_tlv_clear(meshcop_steering_data_tlv);
}

bool
meshcop_steering_data_tlv_is_valid(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv)
{
    return ((tlv_get_length(&meshcop_steering_data_tlv->tlv) != 0) &&
            (tlv_get_length(&meshcop_steering_data_tlv->tlv) <= sizeof(*meshcop_steering_data_tlv) - sizeof(meshcop_tlv_t)));
}

void
meshcop_steering_data_tlv_clear(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv)
{
    memset(meshcop_steering_data_tlv->steering_data, 0, tlv_get_length(&meshcop_steering_data_tlv->tlv));
}

void
meshcop_steering_data_tlv_set(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv)
{
    memset(meshcop_steering_data_tlv->steering_data, 0xff, tlv_get_length(&meshcop_steering_data_tlv->tlv));
}

bool
meshcop_steering_data_tlv_does_allow_any(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv)
{
    bool rval = true;

    for (uint8_t i = 0; i < tlv_get_length(&meshcop_steering_data_tlv->tlv); i++) {
        if (meshcop_steering_data_tlv->steering_data[i] != 0xff) {
            rval = false;
            break;
        }
    }

    return rval;
}

uint8_t
meshcop_steering_data_tlv_get_num_bits(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv)
{
    return tlv_get_length(&meshcop_steering_data_tlv->tlv) * 8;
}

bool
meshcop_steering_data_tlv_get_bit(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv, uint8_t bit)
{
    return (meshcop_steering_data_tlv->steering_data[tlv_get_length(&meshcop_steering_data_tlv->tlv) - 1 - (bit / 8)] & (1 << (bit % 8))) != 0;
}

void
meshcop_steering_data_tlv_clear_bit(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv, uint8_t bit)
{
    meshcop_steering_data_tlv->steering_data[tlv_get_length(&meshcop_steering_data_tlv->tlv) - 1 - (bit / 8)] &= ~(1 << (bit % 8));
}

void
meshcop_steering_data_tlv_set_bit(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv, uint8_t bit)
{
    meshcop_steering_data_tlv->steering_data[tlv_get_length(&meshcop_steering_data_tlv->tlv) - 1 - (bit / 8)] |= (1 << (bit % 8));
}

bool
meshcop_steering_data_tlv_is_cleared(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv)
{
    bool rval = true;

    for (uint8_t i = 0; i < tlv_get_length(&meshcop_steering_data_tlv->tlv); i++) {
        if (meshcop_steering_data_tlv->steering_data[i] != 0) {
            rval = false;
            break;
        }
    }

    return rval;
}

void
meshcop_steering_data_tlv_compute_bloom_filter(meshcop_steering_data_tlv_t *meshcop_steering_data_tlv,
                                               const ns_ext_addr_t joiner_id)
{
    crc16_t ccitt;
    crc16_t ansi;

    crc16_ctor(&ccitt, CRC16_POLYNOMIAL_CCITT);
    crc16_ctor(&ansi, CRC16_POLYNOMIAL_ANSI);

    for (size_t j = 0; j < sizeof(ns_ext_addr_t); j++) {
        uint8_t byte = joiner_id.m8[j];
        crc16_update(&ccitt, byte);
        crc16_update(&ansi, byte);
    }

    meshcop_steering_data_tlv_set_bit(meshcop_steering_data_tlv,
            crc16_get(&ccitt) % meshcop_steering_data_tlv_get_num_bits(meshcop_steering_data_tlv));
    meshcop_steering_data_tlv_set_bit(meshcop_steering_data_tlv,
            crc16_get(&ansi) % meshcop_steering_data_tlv_get_num_bits(meshcop_steering_data_tlv));
}

// --- border agent locator tlv
void
meshcop_border_agent_locator_tlv_init(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv)
{
    meshcop_tlv_set_type(&meshcop_border_agent_locator_tlv->tlv, MESHCOP_TLV_TYPE_BORDER_AGENT_LOCATOR);
    tlv_set_length(&meshcop_border_agent_locator_tlv->tlv, sizeof(*meshcop_border_agent_locator_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_border_agent_locator_tlv_is_valid(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv)
{
    return tlv_get_length(&meshcop_border_agent_locator_tlv->tlv) == sizeof(*meshcop_border_agent_locator_tlv) - sizeof(meshcop_tlv_t);
}

uint16_t
meshcop_border_agent_locator_tlv_get_border_agent_locator(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv)
{
    return encoding_big_endian_swap16(meshcop_border_agent_locator_tlv->locator);
}

void
meshcop_border_agent_locator_tlv_set_border_agent_locator(meshcop_border_agent_locator_tlv_t *meshcop_border_agent_locator_tlv, uint16_t locator)
{
    meshcop_border_agent_locator_tlv->locator = encoding_big_endian_swap16(locator);
}

// --- commissioner id tlv
void
meshcop_commissioner_id_tlv_init(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv)
{
    meshcop_tlv_set_type(&meshcop_commissioner_id_tlv->tlv, MESHCOP_TLV_TYPE_COMMISSIONER_ID);
    tlv_set_length(&meshcop_commissioner_id_tlv->tlv, sizeof(*meshcop_commissioner_id_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_commissioner_id_tlv_is_valid(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv)
{
    return tlv_get_length(&meshcop_commissioner_id_tlv->tlv) <= sizeof(*meshcop_commissioner_id_tlv) - sizeof(meshcop_tlv_t);
}

const char *
meshcop_commissioner_id_tlv_get_commissioner_id(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv)
{
    return meshcop_commissioner_id_tlv->commissioner_id;
}

void
meshcop_commissioner_id_tlv_set_commissioner_id(meshcop_commissioner_id_tlv_t *meshcop_commissioner_id_tlv,
                                                const char *commissioner_id)
{
    size_t length = strnlen(commissioner_id, sizeof(meshcop_commissioner_id_tlv->commissioner_id));
    memcpy(meshcop_commissioner_id_tlv->commissioner_id, commissioner_id, length);
    tlv_set_length(&meshcop_commissioner_id_tlv->tlv, (uint8_t)length);
}

// --- commissioner session id tlv
void
meshcop_commissioner_session_id_tlv_init(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv)
{
    meshcop_tlv_set_type(&meshcop_commissioner_session_id_tlv->tlv, MESHCOP_TLV_TYPE_COMMISSIONER_SESSION_ID);
    tlv_set_length(&meshcop_commissioner_session_id_tlv->tlv, sizeof(*meshcop_commissioner_session_id_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_commissioner_session_id_tlv_is_valid(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv)
{
    return tlv_get_length(&meshcop_commissioner_session_id_tlv->tlv) == sizeof(*meshcop_commissioner_session_id_tlv) - sizeof(meshcop_tlv_t);
}

uint16_t
meshcop_commissioner_session_id_tlv_get_commissioner_session_id(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv)
{
    return encoding_big_endian_swap16(meshcop_commissioner_session_id_tlv->session_id);
}

void
meshcop_commissioner_session_id_tlv_set_commissioner_session_id(meshcop_commissioner_session_id_tlv_t *meshcop_commissioner_session_id_tlv, uint16_t session_id)
{
    meshcop_commissioner_session_id_tlv->session_id = encoding_big_endian_swap16(session_id);
}

// --- security policy tlv
void
meshcop_security_policy_tlv_init(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv)
{
    meshcop_tlv_set_type(&meshcop_security_policy_tlv->tlv, MESHCOP_TLV_TYPE_SECURITY_POLICY);
    tlv_set_length(&meshcop_security_policy_tlv->tlv, sizeof(*meshcop_security_policy_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_security_policy_tlv_is_valid(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv)
{
    return tlv_get_length(&meshcop_security_policy_tlv->tlv) == sizeof(*meshcop_security_policy_tlv) - sizeof(meshcop_tlv_t);
}

uint16_t
meshcop_security_policy_tlv_get_rotation_time(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv)
{
    return encoding_big_endian_swap16(meshcop_security_policy_tlv->rotation_time);
}

void
meshcop_security_policy_tlv_set_rotation_time(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv, uint16_t rotation_time)
{
    meshcop_security_policy_tlv->rotation_time = encoding_big_endian_swap16(rotation_time);
}

uint8_t
meshcop_security_policy_tlv_get_flags(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv)
{
    return meshcop_security_policy_tlv->flags;
}

void
meshcop_security_policy_tlv_set_flags(meshcop_security_policy_tlv_t *meshcop_security_policy_tlv, uint8_t flags)
{
    meshcop_security_policy_tlv->flags = flags;
}

// --- active timestamp tlv
void
meshcop_active_timestamp_tlv_init(meshcop_active_timestamp_tlv_t *meshcop_active_timestamp_tlv)
{
    meshcop_tlv_set_type(&meshcop_active_timestamp_tlv->tlv, MESHCOP_TLV_TYPE_ACTIVE_TIMESTAMP);
    tlv_set_length(&meshcop_active_timestamp_tlv->tlv, sizeof(*meshcop_active_timestamp_tlv) - sizeof(meshcop_tlv_t));
    meshcop_timestamp_init(&meshcop_active_timestamp_tlv->timestamp);
}

bool
meshcop_active_timestamp_tlv_is_valid(meshcop_active_timestamp_tlv_t *meshcop_active_timestamp_tlv)
{
    return tlv_get_length(&meshcop_active_timestamp_tlv->tlv) == sizeof(*meshcop_active_timestamp_tlv) - sizeof(meshcop_tlv_t);
}

// --- state tlv
void
meshcop_state_tlv_init(meshcop_state_tlv_t *meshcop_state_tlv)
{
    meshcop_tlv_set_type(&meshcop_state_tlv->tlv, MESHCOP_TLV_TYPE_STATE);
    tlv_set_length(&meshcop_state_tlv->tlv, sizeof(*meshcop_state_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_state_tlv_is_valid(meshcop_state_tlv_t *meshcop_state_tlv)
{
    return tlv_get_length(&meshcop_state_tlv->tlv) == sizeof(*meshcop_state_tlv) - sizeof(meshcop_tlv_t);
}

meshcop_state_tlv_state_t
meshcop_state_tlv_get_state(meshcop_state_tlv_t *meshcop_state_tlv)
{
    return (meshcop_state_tlv_state_t)meshcop_state_tlv->state;
}

void
meshcop_state_tlv_set_state(meshcop_state_tlv_t *meshcop_state_tlv, meshcop_state_tlv_state_t state)
{
    meshcop_state_tlv->state = (uint8_t)state;
}

// --- joiner udp port tlv functions
void
meshcop_joiner_udp_port_tlv_init(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv)
{
    meshcop_tlv_set_type(&meshcop_joiner_udp_port_tlv->tlv, MESHCOP_TLV_TYPE_JOINER_UDP_PORT);
    tlv_set_length(&meshcop_joiner_udp_port_tlv->tlv, sizeof(*meshcop_joiner_udp_port_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_joiner_udp_port_tlv_is_valid(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv)
{
    return tlv_get_length(&meshcop_joiner_udp_port_tlv->tlv) == sizeof(*meshcop_joiner_udp_port_tlv) - sizeof(meshcop_tlv_t);
}

uint16_t
meshcop_joiner_udp_port_tlv_get_udp_port(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv)
{
    return encoding_big_endian_swap16(meshcop_joiner_udp_port_tlv->udp_port);
}

void
meshcop_joiner_udp_port_tlv_set_udp_port(meshcop_joiner_udp_port_tlv_t *meshcop_joiner_udp_port_tlv, uint16_t udp_port)
{
    meshcop_joiner_udp_port_tlv->udp_port = encoding_big_endian_swap16(udp_port);
}

// --- joiner iid tlv functions
void
meshcop_joiner_iid_tlv_init(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv)
{
    meshcop_tlv_set_type(&meshcop_joiner_iid_tlv->tlv, MESHCOP_TLV_TYPE_JOINER_IID);
    tlv_set_length(&meshcop_joiner_iid_tlv->tlv, sizeof(*meshcop_joiner_iid_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_joiner_iid_tlv_is_valid(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv)
{
    return tlv_get_length(&meshcop_joiner_iid_tlv->tlv) == sizeof(*meshcop_joiner_iid_tlv) - sizeof(meshcop_tlv_t);
}

const uint8_t *
meshcop_joiner_iid_tlv_get_iid(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv)
{
    return meshcop_joiner_iid_tlv->iid;
}

void
meshcop_joiner_iid_tlv_set_iid(meshcop_joiner_iid_tlv_t *meshcop_joiner_iid_tlv, const uint8_t *iid)
{
    memcpy(meshcop_joiner_iid_tlv->iid, iid, sizeof(meshcop_joiner_iid_tlv->iid));
}

// --- joiner router locator tlv functions
void
meshcop_joiner_router_locator_tlv_init(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv)
{
    meshcop_tlv_set_type(&meshcop_joiner_router_locator_tlv->tlv, MESHCOP_TLV_TYPE_JOINER_ROUTER_LOCATOR);
    tlv_set_length(&meshcop_joiner_router_locator_tlv->tlv, sizeof(*meshcop_joiner_router_locator_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_joiner_router_locator_tlv_is_valid(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv)
{
    return tlv_get_length(&meshcop_joiner_router_locator_tlv->tlv) == sizeof(*meshcop_joiner_router_locator_tlv) - sizeof(meshcop_tlv_t);
}

uint16_t
meshcop_joiner_router_locator_tlv_get_joiner_router_locator(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv)
{
    return encoding_big_endian_swap16(meshcop_joiner_router_locator_tlv->locator);
}

void
meshcop_joiner_router_locator_tlv_set_joiner_router_locator(meshcop_joiner_router_locator_tlv_t *meshcop_joiner_router_locator_tlv, uint16_t locator)
{
    meshcop_joiner_router_locator_tlv->locator = encoding_big_endian_swap16(locator);
}

// --- joiner router kek tlv functions
void
meshcop_joiner_router_kek_tlv_init(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv)
{
    meshcop_tlv_set_type(&meshcop_joiner_router_kek_tlv->tlv, MESHCOP_TLV_TYPE_JOINER_ROUTER_KEK);
    tlv_set_length(&meshcop_joiner_router_kek_tlv->tlv, sizeof(*meshcop_joiner_router_kek_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_joiner_router_kek_tlv_is_valid(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv)
{
    return tlv_get_length(&meshcop_joiner_router_kek_tlv->tlv) == sizeof(*meshcop_joiner_router_kek_tlv) - sizeof(meshcop_tlv_t);
}

const uint8_t *
meshcop_joiner_router_kek_tlv_get_kek(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv)
{
    return meshcop_joiner_router_kek_tlv->kek;
}

void
meshcop_joiner_router_kek_tlv_set_kek(meshcop_joiner_router_kek_tlv_t *meshcop_joiner_router_kek_tlv, const uint8_t *kek)
{
    memcpy(&meshcop_joiner_router_kek_tlv->kek, kek, sizeof(meshcop_joiner_router_kek_tlv->kek));
}

// --- pending timestamp tlv functions
void
meshcop_pending_timestamp_tlv_init(meshcop_pending_timestamp_tlv_t *meshcop_pending_timestamp_tlv)
{
    meshcop_tlv_set_type(&meshcop_pending_timestamp_tlv->tlv, MESHCOP_TLV_TYPE_PENDING_TIMESTAMP);
    tlv_set_length(&meshcop_pending_timestamp_tlv->tlv, sizeof(*meshcop_pending_timestamp_tlv) - sizeof(meshcop_tlv_t));
    meshcop_timestamp_init(&meshcop_pending_timestamp_tlv->timestamp);
}

bool
meshcop_pending_timestamp_tlv_is_valid(meshcop_pending_timestamp_tlv_t *meshcop_pending_timestamp_tlv)
{
    return tlv_get_length(&meshcop_pending_timestamp_tlv->tlv) == sizeof(*meshcop_pending_timestamp_tlv) - sizeof(meshcop_tlv_t);
}

// --- delay timer tlv functions
void
meshcop_delay_timer_tlv_init(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv)
{
    meshcop_tlv_set_type(&meshcop_delay_timer_tlv->tlv, MESHCOP_TLV_TYPE_DELAY_TIMER);
    tlv_set_length(&meshcop_delay_timer_tlv->tlv, sizeof(*meshcop_delay_timer_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_delay_timer_tlv_is_valid(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv)
{
    return tlv_get_length(&meshcop_delay_timer_tlv->tlv) == sizeof(*meshcop_delay_timer_tlv) - sizeof(meshcop_tlv_t);
}

uint32_t
meshcop_delay_timer_tlv_get_delay_timer(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv)
{
    return encoding_big_endian_swap32(meshcop_delay_timer_tlv->delay_timer);
}

void
meshcop_delay_timer_tlv_set_delay_timer(meshcop_delay_timer_tlv_t *meshcop_delay_timer_tlv, uint32_t delay_timer)
{
    meshcop_delay_timer_tlv->delay_timer = encoding_big_endian_swap32(delay_timer);
}

// --- channel mask entry base tlv functions
uint8_t
meshcop_channel_mask_entry_base_get_channel_page(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base)
{
    return meshcop_channel_mask_entry_base->channel_page;
}

void
meshcop_channel_mask_entry_base_set_channel_page(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base,
                                                     uint8_t channel_page)
{
    meshcop_channel_mask_entry_base->channel_page = channel_page;
}

uint8_t
meshcop_channel_mask_entry_base_get_mask_length(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base)
{
    return meshcop_channel_mask_entry_base->mask_length;
}

void
meshcop_channel_mask_entry_base_set_mask_length(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base, uint8_t mask_length)
{
    meshcop_channel_mask_entry_base->mask_length = mask_length;
}

uint16_t
meshcop_channel_mask_entry_base_get_size(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base)
{
    return sizeof(meshcop_channel_mask_entry_base_t) + meshcop_channel_mask_entry_base->mask_length;
}

void
meshcop_channel_mask_entry_base_clear_channel(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base, uint8_t channel)
{
    uint8_t *mask = (uint8_t *)meshcop_channel_mask_entry_base + sizeof(*meshcop_channel_mask_entry_base);
    mask[channel / 8] &= ~(0x80 >> (channel % 8));
}

bool
meshcop_channel_mask_entry_base_is_channel_set(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base, uint8_t channel)
{
    const uint8_t *mask = (const uint8_t *)meshcop_channel_mask_entry_base + sizeof(*meshcop_channel_mask_entry_base);
    return (channel << (meshcop_channel_mask_entry_base->mask_length * 8)) ? ((mask[channel / 8] & (0x80 >> (channel % 8))) != 0) : false;
}

const meshcop_channel_mask_entry_base_t *
meshcop_channel_mask_entry_base_get_next(meshcop_channel_mask_entry_base_t *meshcop_channel_mask_entry_base, const meshcop_tlv_t *channel_mask_base_tlv)
{
    const uint8_t *entry = (const uint8_t *)meshcop_channel_mask_entry_base + meshcop_channel_mask_entry_base_get_size(meshcop_channel_mask_entry_base);
    const uint8_t *end = tlv_get_value((tlv_t *)channel_mask_base_tlv) + tlv_get_size((tlv_t *)channel_mask_base_tlv);

    return (entry < end) ? (const meshcop_channel_mask_entry_base_t *)entry : NULL;
}

// --- channel mask entry functions
void
meshcop_channel_mask_entry_init(meshcop_channel_mask_entry_t *meshcop_channel_mask_entry)
{
    meshcop_channel_mask_entry_base_set_channel_page(&meshcop_channel_mask_entry->channel_mask_entry_base, 0);
    meshcop_channel_mask_entry_base_set_mask_length(&meshcop_channel_mask_entry->channel_mask_entry_base, sizeof(meshcop_channel_mask_entry->mask));
}

bool
meshcop_channel_mask_entry_is_valid(meshcop_channel_mask_entry_t *meshcop_channel_mask_entry)
{
    return meshcop_channel_mask_entry_base_get_mask_length(&meshcop_channel_mask_entry->channel_mask_entry_base) == sizeof(meshcop_channel_mask_entry->mask);
}

uint32_t
meshcop_channel_mask_entry_get_mask(meshcop_channel_mask_entry_t *meshcop_channel_mask_entry)
{
    return encoding_reverse32(encoding_big_endian_swap32(meshcop_channel_mask_entry->mask));
}

void
meshcop_channel_mask_entry_set_mask(meshcop_channel_mask_entry_t *meshcop_channel_mask_entry, uint32_t mask)
{
    meshcop_channel_mask_entry->mask = encoding_big_endian_swap32(encoding_reverse32(mask));
}

// --- channel mask base tlv functions
void
meshcop_channel_mask_base_tlv_init(meshcop_channel_mask_base_tlv_t *meshcop_channel_mask_base_tlv)
{
    meshcop_tlv_set_type(&meshcop_channel_mask_base_tlv->tlv, MESHCOP_TLV_TYPE_CHANNEL_MASK);
    tlv_set_length(&meshcop_channel_mask_base_tlv->tlv, sizeof(*meshcop_channel_mask_base_tlv) - sizeof(meshcop_tlv_t));
}

bool
meshcop_channel_mask_base_tlv_is_valid(meshcop_channel_mask_base_tlv_t *meshcop_channel_mask_base_tlv)
{
    return true;
}

const meshcop_channel_mask_entry_base_t *
meshcop_channel_mask_base_tlv_get_first_entry(meshcop_channel_mask_base_tlv_t *meshcop_channel_mask_base_tlv)
{
    const meshcop_channel_mask_entry_base_t *entry = NULL;

    VERIFY_OR_EXIT(tlv_get_length(&meshcop_channel_mask_base_tlv->tlv) >= sizeof(meshcop_channel_mask_entry_base_t));

    entry = (const meshcop_channel_mask_entry_base_t *)tlv_get_value(&meshcop_channel_mask_base_tlv->tlv);
    VERIFY_OR_EXIT(tlv_get_length(&meshcop_channel_mask_base_tlv->tlv) >= tlv_get_size(&meshcop_channel_mask_base_tlv->tlv), entry = NULL);

exit:
    return entry;
}

const meshcop_channel_mask_entry_t *
meshcop_channel_mask_base_tlv_get_mask_entry(meshcop_channel_mask_base_tlv_t *meshcop_channel_mask_base_tlv,
                                             uint8_t channel_page)
{
    const meshcop_channel_mask_entry_t *page_entry = NULL;

    for (const meshcop_channel_mask_entry_base_t *entry = meshcop_channel_mask_base_tlv_get_first_entry(meshcop_channel_mask_base_tlv);
         entry != NULL;
         entry = meshcop_channel_mask_entry_base_get_next((meshcop_channel_mask_entry_base_t *)entry, (const meshcop_tlv_t *)meshcop_channel_mask_base_tlv)) {
        if (meshcop_channel_mask_entry_base_get_channel_page((meshcop_channel_mask_entry_base_t *)entry) == channel_page) {
            page_entry = (const meshcop_channel_mask_entry_t *)entry;
            if (meshcop_channel_mask_entry_is_valid((meshcop_channel_mask_entry_t *)page_entry)) {
                EXIT_NOW();
            }
        }
    }

    page_entry = NULL;

exit:
    return page_entry;
}
