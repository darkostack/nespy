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
