#include "core/meshcop/timestamp.h"

void
meshcop_timestamp_init(meshcop_timestamp_t *meshcop_timestamp)
{
    memset(meshcop_timestamp->seconds, 0, sizeof(meshcop_timestamp->seconds));
    meshcop_timestamp->ticks = 0;
}

int
meshcop_timestamp_compare(meshcop_timestamp_t *meshcop_timestamp, const meshcop_timestamp_t *compare)
{
    uint64_t this_seconds = meshcop_timestamp_get_seconds(meshcop_timestamp);
    uint64_t compare_seconds = meshcop_timestamp_get_seconds((meshcop_timestamp_t *)compare);
    uint16_t this_ticks = meshcop_timestamp_get_ticks(meshcop_timestamp);
    uint16_t compare_ticks = meshcop_timestamp_get_ticks((meshcop_timestamp_t *)compare);
    int rval;

    if (compare_seconds > this_seconds) {
        rval = 1;
    } else if (compare_seconds < this_seconds) {
        rval = -1;
    } else if (compare_ticks > this_ticks) {
        rval = 1;
    } else if (compare_ticks < this_ticks) {
        rval = -1;
    } else {
        rval = 0;
    }

    return rval;
}

uint64_t
meshcop_timestamp_get_seconds(meshcop_timestamp_t *meshcop_timestamp)
{
    uint64_t seconds = 0;

    for (size_t i = 0; i < sizeof(meshcop_timestamp->seconds); i++) {
        seconds = (seconds << 8) | meshcop_timestamp->seconds[i];
    }

    return seconds;
}

void
meshcop_timestamp_set_seconds(meshcop_timestamp_t *meshcop_timestamp, uint64_t seconds)
{
    for (size_t i = 0; i < sizeof(meshcop_timestamp->seconds); i++, seconds >>= 8) {
        meshcop_timestamp->seconds[sizeof(meshcop_timestamp->seconds) - 1 - i] = seconds & 0xff;
    }
}

uint16_t
meshcop_timestamp_get_ticks(meshcop_timestamp_t *meshcop_timestamp)
{
    return encoding_big_endian_swap16(meshcop_timestamp->ticks) >> MESHCOP_TIMESTAMP_TICKS_OFFSET;
}

void
meshcop_timestamp_set_ticks(meshcop_timestamp_t *meshcop_timestamp, uint16_t ticks)
{
    meshcop_timestamp->ticks = encoding_big_endian_swap16((encoding_big_endian_swap16(meshcop_timestamp->ticks) & ~MESHCOP_TIMESTAMP_TICKS_MASK) |
            ((ticks << MESHCOP_TIMESTAMP_TICKS_OFFSET) & MESHCOP_TIMESTAMP_TICKS_MASK));
}

bool
meshcop_timestamp_get_authoritative(meshcop_timestamp_t *meshcop_timestamp)
{
    return (encoding_big_endian_swap16(meshcop_timestamp->ticks) & MESHCOP_AUTHORITATIVE_MASK) != 0;
}

void
meshcop_timestamp_set_authoritative(meshcop_timestamp_t *meshcop_timestamp, bool authoritative)
{
    meshcop_timestamp->ticks = encoding_big_endian_swap16((encoding_big_endian_swap16(meshcop_timestamp->ticks) & MESHCOP_TIMESTAMP_TICKS_MASK) |
            ((authoritative << MESHCOP_AUTHORITATIVE_OFFSET) & MESHCOP_AUTHORITATIVE_MASK));
}
