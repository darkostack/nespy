#ifndef NS_CORE_UTILS_HEAP_H_
#define NS_CORE_UTILS_HEAP_H_

#include "ns/sys/core/core-config.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct _block block_t;
typedef struct _heap heap_t;

enum {
    HEAP_GUARD_BLOCK_SIZE = 0xffff,
};

/**
 * This struct represents a memory block.
 *
 * A block is of the structure as below.
 *
 *     +------------------------------+
 *     | Size    |  Memory  |  Next   |
 *     |---------|----------| --------|
 *     | 2 bytes |  n bytes | 2 bytes |
 *     +------------------------------+
 *
 * Since block metadata is of 4-byte size, mSize and mNext are separated at the beginning
 * and end of the block to make sure the mMemory is aligned with long.
 *
 */
struct _block {
    uint16_t size;
    uint8_t memory[sizeof(uint16_t)];
};

enum {
    HEAP_MEMORY_SIZE          = NS_CONFIG_HEAP_SIZE,
    HEAP_ALIGN_SIZE           = sizeof(long),
    HEAP_BLOCK_REMAINDER_SIZE = HEAP_ALIGN_SIZE - sizeof(uint16_t) * 2,
    HEAP_SUPER_BLOCK_SIZE     = HEAP_ALIGN_SIZE - sizeof(block_t),
    HEAP_FIRST_BLOCK_SIZE     = HEAP_MEMORY_SIZE - HEAP_ALIGN_SIZE * 3 + HEAP_BLOCK_REMAINDER_SIZE,
    HEAP_SUPER_BLOCK_OFFSET   = HEAP_ALIGN_SIZE - sizeof(uint16_t),
    HEAP_FIRST_BLOCK_OFFSET   = HEAP_ALIGN_SIZE * 2 - sizeof(uint16_t),
    HEAP_GUARD_BLOCK_OFFSET   = HEAP_MEMORY_SIZE - sizeof(uint16_t),
};

/**
 * This struct defines functionality to manipulate heap.
 *
 * This implementation is currently for mbedTLS.
 *
 * The memory is divided into blocks. The whole picture is as follows:
 *
 *     +--------------------------------------------------------------------------+
 *     |    unused      |    super   | block 1 | block 2 | ... | block n | guard  |
 *     +----------------+------------+---------+---------+-----+---------+--------+
 *     | kAlignSize - 2 | kAlignSize | 4 + s1  | 4 + s2  | ... | 4 + s4  |   2    |
 *     +--------------------------------------------------------------------------+
 *
 */
struct _heap {
    union {
        uint16_t free_size;
        // make sure memory is long aligned.
        long typelong[HEAP_MEMORY_SIZE / sizeof(long)];
        uint8_t type8[HEAP_MEMORY_SIZE];
        uint16_t type16[HEAP_MEMORY_SIZE / sizeof(uint16_t)];
    } memory;
};

void
heap_make_new(heap_t *heap);

void *
heap_calloc(heap_t *heap, size_t count, size_t asize);

void
heap_free(heap_t *heap, void *ptr);

bool
heap_is_clean(heap_t *heap);

size_t
heap_get_capacity(void);

size_t
heap_get_free_size(heap_t *heap);

#endif // NS_CORE_UTILS_HEAP_H_
