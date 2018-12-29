#ifndef NS_CORE_COMMON_TLVS_H_
#define NS_CORE_COMMON_TLVS_H_

#include "core/core-config.h"
#include "core/common/encoding.h"
#include "core/common/message.h"
#include <string.h>

enum {
    TLV_EXTENDED_LENGTH = 255,
};

typedef struct _tlv {
    uint8_t type;
    uint8_t length;
} tlv_t;

typedef struct _extended_tlv {
    tlv_t tlv;
    uint16_t length;
} extended_tlv_t;

// --- tlv functions
uint8_t
tlv_get_type(tlv_t *tlv);

void
tlv_set_type(tlv_t *tlv, uint8_t type);

uint8_t
tlv_get_length(tlv_t *tlv);

void
tlv_set_length(tlv_t *tlv, uint8_t length);

uint16_t
tlv_get_size(tlv_t *tlv);

uint8_t *
tlv_get_value(tlv_t *tlv);

tlv_t *
tlv_get_next(tlv_t *tlv);

ns_error_t
tlv_get(const message_t message, uint8_t type, uint16_t max_length, tlv_t *tlv);

ns_error_t
tlv_get_offset(const message_t message, uint8_t type, uint16_t *aoffset);

ns_error_t
tlv_get_value_offset(const message_t message, uint8_t type, uint16_t *aoffset, uint16_t *alength);

// --- extended tlv functions
uint16_t
extended_tlv_get_length(extended_tlv_t *extended_tlv);

void
extended_tlv_set_length(extended_tlv_t *extended_tlv, uint16_t length);

#endif // NS_CORE_COMMON_TLVS_H_
