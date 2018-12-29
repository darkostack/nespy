#include "core/meshcop/meshcop_tlvs.h"

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
