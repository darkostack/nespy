#ifndef NS_CORE_MESHCOP_TIMESTAMP_H_
#define NS_CORE_MESHCOP_TIMESTAMP_H_

#include "core/core-config.h"
#include "core/common/encoding.h"
#include <string.h>
#include <stdbool.h>

enum {
    MESHCOP_TIMESTAMP_TICKS_OFFSET = 1,
    MESHCOP_TIMESTAMP_TICKS_MASK   = 0x7fff << MESHCOP_TIMESTAMP_TICKS_OFFSET,
    MESHCOP_AUTHORITATIVE_OFFSET   = 0,
    MESHCOP_AUTHORITATIVE_MASK     = 1 << MESHCOP_AUTHORITATIVE_OFFSET,
};

typedef struct _meshcop_timestamp {
    uint8_t seconds[6];
    uint16_t ticks;
} meshcop_timestamp_t;

// --- meshcop timestamp functions
void
meshcop_timestamp_init(meshcop_timestamp_t *meshcop_timestamp);

int
meshcop_timestamp_compare(meshcop_timestamp_t *meshcop_timestamp, const meshcop_timestamp_t *compare);

uint64_t
meshcop_timestamp_get_seconds(meshcop_timestamp_t *meshcop_timestamp);

void
meshcop_timestamp_set_seconds(meshcop_timestamp_t *meshcop_timestamp, uint64_t seconds);

uint16_t
meshcop_timestamp_get_ticks(meshcop_timestamp_t *meshcop_timestamp);

void
meshcop_timestamp_set_ticks(meshcop_timestamp_t *meshcop_timestamp, uint16_t ticks);

bool
meshcop_timestamp_get_authoritative(meshcop_timestamp_t *meshcop_timestamp);

void
meshcop_timestamp_set_authoritative(meshcop_timestamp_t *meshcop_timestamp, bool authoritative);

#endif // NS_CORE_MESHCOP_TIMESTAMP_H_
