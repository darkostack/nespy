#include "core/common/tlvs.h"
#include "core/common/code_utils.h"

// --- tlv functions
uint8_t
tlv_get_type(tlv_t *tlv)
{
    return tlv->type;
}

void
tlv_set_type(tlv_t *tlv, uint8_t type)
{
    tlv->type = type;
}

uint8_t
tlv_get_length(tlv_t *tlv)
{
    return tlv->length;
}

void
tlv_set_length(tlv_t *tlv, uint8_t length)
{
    tlv->length = length;
}

uint16_t
tlv_get_size(tlv_t *tlv)
{
    return sizeof(tlv_t) + tlv->length;
}

uint8_t *
tlv_get_value(tlv_t *tlv)
{
    return (uint8_t *)tlv + sizeof(tlv_t);
}

tlv_t *
tlv_get_next(tlv_t *tlv)
{
    return (tlv_t *)((uint8_t *)tlv + sizeof(*tlv) + tlv->length);
}

ns_error_t
tlv_get(const message_t message, uint8_t type, uint16_t max_length, tlv_t *tlv)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    uint16_t offset;

    SUCCESS_OR_EXIT(error = tlv_get_offset(message, type, &offset));
    message_read(message, offset, tlv, sizeof(tlv_t));

    if (max_length > sizeof(*tlv) + tlv_get_length(tlv)) {
        max_length = sizeof(*tlv) + tlv_get_length(tlv);
    }

    message_read(message, offset, tlv, max_length);

exit:
    return error;
}

ns_error_t
tlv_get_offset(const message_t message, uint8_t type, uint16_t *aoffset)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    uint16_t offset = message_get_offset(message);
    uint16_t end = message_get_length(message);
    tlv_t tlv;

    while (offset + sizeof(tlv) <= end) {
        uint32_t length = sizeof(tlv);

        message_read(message, offset, &tlv, sizeof(tlv));

        if (tlv_get_length(&tlv) != TLV_EXTENDED_LENGTH) {
            length += tlv_get_length(&tlv);
        } else {
            uint16_t ext_length;
            VERIFY_OR_EXIT(sizeof(ext_length) == message_read(message, offset + sizeof(tlv), &ext_length, sizeof(ext_length)));
            length += sizeof(ext_length) + encoding_big_endian_swap16(ext_length);
        }

        VERIFY_OR_EXIT(offset + length <= end);

        if (tlv_get_type(&tlv) == type) {
            *aoffset = offset;
            EXIT_NOW(error = NS_ERROR_NONE);
        }

        offset += (uint16_t)length;
    }

exit:
    return error;
}

ns_error_t
tlv_get_value_offset(const message_t message, uint8_t type, uint16_t *aoffset, uint16_t *alength)
{
    ns_error_t error = NS_ERROR_NOT_FOUND;
    uint16_t offset = message_get_offset(message);
    uint16_t end = message_get_length(message);
    tlv_t tlv;

    while (offset + sizeof(tlv) <= end) {
        uint16_t length;

        message_read(message, offset, &tlv, sizeof(tlv));
        offset += sizeof(tlv);
        length = tlv_get_length(&tlv);

        if (length == TLV_EXTENDED_LENGTH) {
            VERIFY_OR_EXIT(offset + sizeof(length) <= end);
            message_read(message, offset, &length, sizeof(length));
            offset += sizeof(length);
            length = encoding_big_endian_swap16(length);
        }

        VERIFY_OR_EXIT(length <= end - offset);

        if (tlv_get_type(&tlv) == type) {
            *aoffset = offset;
            *alength = length;
            EXIT_NOW(error = NS_ERROR_NONE);
        }

        offset += length;
    }

exit:
    return error;
}

// --- extended tlv functions
uint16_t
extended_tlv_get_length(extended_tlv_t *extended_tlv)
{
    return encoding_big_endian_swap16(extended_tlv->length);
}

void
extended_tlv_set_length(extended_tlv_t *extended_tlv, uint16_t length)
{
    tlv_set_length(&extended_tlv->tlv, TLV_EXTENDED_LENGTH);
    extended_tlv->length = encoding_big_endian_swap16(length);
}
