#include "ns/platform/random.h"
#include "core/common/random.h"

uint8_t
random_get_uint8(void)
{
    return (uint8_t)(ns_plat_random_get() & 0xff);
}

uint16_t
random_get_uint16(void)
{
    return (uint16_t)(ns_plat_random_get() & 0xffff);
}

uint32_t
random_get_uint32(void)
{
    return ns_plat_random_get();
}

uint8_t
random_get_uint8_in_range(uint8_t min, uint8_t max)
{
    return (min + (random_get_uint8() % (max - min)));
}

uint16_t
random_get_uint16_in_range(uint16_t min, uint16_t max)
{
    return (min + (random_get_uint16() % (max - min)));
}

uint32_t
random_get_uint32_in_range(uint32_t min, uint32_t max)
{
    return (min + (random_get_uint32() % (max - min)));
}

void
random_fill_buffer(uint8_t *buffer, uint16_t size)
{
    while (size-- != 0) {
        *buffer++ = random_get_uint8();
    }
}

uint32_t
random_add_jitter(uint32_t value, uint16_t jitter)
{
    jitter = (jitter <= value) ? jitter : (uint16_t)value;
    return value + random_get_uint32_in_range(0 , 2 * jitter + 1) - jitter;
}
