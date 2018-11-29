#include <limits.h>
#include <stdint.h>
#include "ns/sys/core/common/encoding.h"

// Note: this implementation is for LITTLE_ENDIAN BYTE_ORDER

uint16_t
encoding_swap16(uint16_t v)
{
    return (((v & 0x00ffu) << 8) & 0xff00) | (((v & 0xff00u) >> 8) & 0x00ff);
}

uint32_t
encoding_swap32(uint32_t v)
{
    return ((v & (uint32_t)(0x000000fful)) << 24) | ((v & (uint32_t)(0x0000ff00ul)) << 8) |
           ((v & (uint32_t)(0x00ff0000ul)) >> 8) | ((v & (uint32_t)(0xff000000ul)) >> 24);
}

uint64_t
encoding_swap64(uint64_t v)
{
    return ((v & (uint64_t)(0x00000000000000ffull)) << 56) |
           ((v & (uint64_t)(0x000000000000ff00ull)) << 40) |
           ((v & (uint64_t)(0x0000000000ff0000ull)) << 24) |
           ((v & (uint64_t)(0x00000000ff000000ull)) << 8) |
           ((v & (uint64_t)(0x000000ff00000000ull)) >> 8) |
           ((v & (uint64_t)(0x0000ff0000000000ull)) >> 24) |
           ((v & (uint64_t)(0x00ff000000000000ull)) >> 40) |
           ((v & (uint64_t)(0xff00000000000000ull)) >> 56);
}

uint32_t
encoding_reverse32(uint32_t v)
{
    v = ((v & 0x55555555) << 1) | ((v & 0xaaaaaaaa) >> 1);
    v = ((v & 0x33333333) << 2) | ((v & 0xcccccccc) >> 2);
    v = ((v & 0x0f0f0f0f) << 4) | ((v & 0xf0f0f0f0) >> 4);
    v = ((v & 0x00ff00ff) << 8) | ((v & 0xff00ff00) >> 8);
    v = ((v & 0x0000ffff) << 16) | ((v & 0xffff0000) >> 16);

    return v;
}

// --- BIG ENDIAN
uint16_t
encoding_big_endian_swap16(uint16_t v)
{
    return encoding_swap16(v);
}

uint32_t
encoding_big_endian_swap32(uint32_t v)
{
    return encoding_swap32(v);
}

uint64_t
encoding_big_endian_swap64(uint64_t v)
{
    return encoding_swap64(v);
}

uint16_t
encoding_big_endian_read_uint16(const uint8_t *buffer)
{
    return (uint16_t)((buffer[0] << 8) | buffer[1]);
}

uint32_t
encoding_big_endian_read_uint32(const uint8_t *buffer)
{
    return ((uint32_t)(buffer[0] << 24) | (uint32_t)(buffer[1] << 16) |
            (uint32_t)(buffer[2] << 8) | (uint32_t)(buffer[3] << 0));
}

void
encoding_big_endian_write_uint16(uint16_t value, uint8_t *buffer)
{
    buffer[0] = (value >> 8) & 0xff;
    buffer[1] = (value >> 0) & 0xff;
}

void
encoding_big_endian_write_uint32(uint32_t value, uint8_t *buffer)
{
    buffer[0] = (value >> 24) & 0xff;
    buffer[1] = (value >> 16) & 0xff;
    buffer[2] = (value >> 8) & 0xff;
    buffer[3] = (value >> 0) & 0xff;
}

// --- LITTLE ENDIAN
uint16_t
encoding_little_endian_swap16(uint16_t v)
{
    return v;
}

uint32_t
encoding_litlle_endian_swap32(uint32_t v)
{
    return v;
}

uint64_t
encoding_little_endian_swap64(uint64_t v)
{
    return v;
}

uint16_t
encoding_little_endian_read_uint16(const uint8_t *buffer)
{
    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

uint32_t
encoding_little_endian_read_uint32(const uint8_t *buffer)
{
    return ((uint32_t)(buffer[0] << 0) | (uint32_t)(buffer[1] << 8) |
            (uint32_t)(buffer[2] << 16) | (uint32_t)(buffer[3] << 24));

}

void
encoding_little_endian_write_uint16(uint16_t value, uint8_t *buffer)
{
    buffer[0] = (value >> 0) & 0xff;
    buffer[1] = (value >> 8) & 0xff;
}

void
encoding_little_endian_write_uint32(uint32_t value, uint8_t *buffer)
{
    buffer[0] = (value >> 0) & 0xff;
    buffer[1] = (value >> 8) & 0xff;
    buffer[2] = (value >> 16) & 0xff;
    buffer[3] = (value >> 24) & 0xff;
}
