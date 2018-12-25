#ifndef NS_CORE_COMMON_ENCODING_H_
#define NS_CORE_COMMON_ENCODING_H_

#include "core/core-config.h"
#include <stdint.h>

#define BIT_VECTOR_BYTES(x) (((x) + (CHAR_BIT - 1)) / CHAR_BIT)

uint16_t
encoding_swap16(uint16_t v);

uint32_t
encoding_swap32(uint32_t v);

uint64_t
encoding_swap64(uint64_t v);

uint32_t
encoding_reverse32(uint32_t v);

// --- BIG ENDIAN
uint16_t
encoding_big_endian_swap16(uint16_t v);

uint32_t
encoding_big_endian_swap32(uint32_t v);

uint64_t
encoding_big_endian_swap64(uint64_t v);

uint16_t
encoding_big_endian_read_uint16(const uint8_t *buffer);

uint32_t
encoding_big_endian_read_uint32(const uint8_t *buffer);

void
encoding_big_endian_write_uint16(uint16_t value, uint8_t *buffer);

void
encoding_big_endian_write_uint32(uint32_t value, uint8_t *buffer);

// --- LITTLE ENDIAN
uint16_t
encoding_little_endian_swap16(uint16_t v);

uint32_t
encoding_litlle_endian_swap32(uint32_t v);

uint64_t
encoding_little_endian_swap64(uint64_t v);

uint16_t
encoding_little_endian_read_uint16(const uint8_t *buffer);

uint32_t
encoding_little_endian_read_uint32(const uint8_t *buffer);

void
encoding_little_endian_write_uint16(uint16_t value, uint8_t *buffer);

void
encoding_little_endian_write_uint32(uint32_t value, uint8_t *buffer);

#endif // NS_CORE_COMMON_ENCODING_H_
