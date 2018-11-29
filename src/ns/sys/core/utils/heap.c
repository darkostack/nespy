#include "ns/sys/core/common/instance.h"
#include <string.h>

// --- private functions declarations
static uint16_t
block_get_size(block_t *block);

static void
block_set_size(block_t *block, uint16_t size);

static uint16_t
block_get_next(block_t *block);

static void
block_set_next(block_t *block, uint16_t next);

static void *
block_get_pointer(block_t *block);

static uint16_t
block_get_left_next(block_t *block);

static bool
block_is_left_free(block_t *block);

static bool
block_is_free(block_t *block);

static block_t *
heap_block_at(heap_t *heap, uint16_t offset);

static block_t *
heap_block_of(heap_t *heap, void *ptr);

static block_t *
heap_block_super(heap_t *heap);

static block_t *
heap_block_next(heap_t *heap, block_t *block);

static block_t *
heap_block_right(heap_t *heap, block_t *block);

static block_t *
heap_block_prev(heap_t *heap, block_t *block);

static bool
heap_is_left_free(heap_t *heap, block_t *block);

static uint16_t
heap_block_offset(heap_t *heap, block_t *block);

static void
heap_block_insert(heap_t *heap, block_t *aprev, block_t *ablock);

// --- heap functions
void
heap_ctor(heap_t *heap)
{
    block_t *super = heap_block_at(heap, HEAP_SUPER_BLOCK_OFFSET);
    block_set_size(super, HEAP_SUPER_BLOCK_SIZE);

    block_t *first = heap_block_right(heap, super);
    block_set_size(first, HEAP_FIRST_BLOCK_SIZE);

    block_t *guard = heap_block_right(heap, first);
    block_set_size(guard, HEAP_GUARD_BLOCK_SIZE);

    block_set_next(super, heap_block_offset(heap, first));
    block_set_next(first, heap_block_offset(heap, guard));

    heap->memory.free_size = HEAP_FIRST_BLOCK_SIZE;
}

void *
heap_calloc(heap_t *heap, size_t count, size_t asize)
{
    void *ret = NULL;
    block_t *prev = NULL;
    block_t *curr = NULL;
    uint16_t size = (uint16_t)(count * asize);

    VERIFY_OR_EXIT(size);

    size += HEAP_ALIGN_SIZE - 1 - HEAP_BLOCK_REMAINDER_SIZE;
    size &= ~(HEAP_ALIGN_SIZE - 1);
    size += HEAP_BLOCK_REMAINDER_SIZE;

    prev = heap_block_super(heap);
    curr = heap_block_next(heap, prev);

    while (block_get_size(curr) < size) {
        prev = curr;
        curr = heap_block_next(heap, curr);
    }

    VERIFY_OR_EXIT(block_is_free(curr));

    block_set_next(prev, block_get_next(curr));

    if (block_get_size(curr) > size + sizeof(block_t)) {
        uint16_t new_block_size = block_get_size(curr) - size - sizeof(block_t);
        block_set_size(curr, size);

        block_t *new_block = heap_block_right(heap, curr);
        block_set_size(new_block, new_block_size);
        block_set_next(new_block, 0);

        if (block_get_size(prev) < new_block_size) {
            heap_block_insert(heap, prev, new_block);
        } else {
            heap_block_insert(heap, heap_block_super(heap), new_block);
        }

        heap->memory.free_size -= sizeof(block_t);
    }

    heap->memory.free_size -= block_get_size(curr);

    block_set_next(curr, 0);

    memset(block_get_pointer(curr), 0, size);
    ret = block_get_pointer(curr);

exit:
    return ret;
}

void
heap_free(heap_t *heap, void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    block_t *block = heap_block_of(heap, ptr);
    block_t *right = heap_block_right(heap, block);

    heap->memory.free_size += block_get_size(block);

    if (heap_is_left_free(heap, block)) {

        block_t *prev = heap_block_super(heap);
        block_t *left = heap_block_next(heap, prev);

        heap->memory.free_size += sizeof(block_t);

        for (uint16_t offset = block_get_left_next(block);
             block_get_next(left) != offset;
             left = heap_block_next(heap, left)) {
            prev = left;
        }

        // remove left from free list
        block_set_next(prev, block_get_next(left));
        block_set_next(left, 0);

        if (block_is_free(right)) {
            heap->memory.free_size += sizeof(block_t);

            if (block_get_size(right) > block_get_size(left)) {
                for (uint16_t offset = heap_block_offset(heap, right);
                     block_get_next(prev) != offset;
                     prev = heap_block_next(heap, prev)) {
                    // do nothing
                }
            } else {
                prev = heap_block_prev(heap, right);
            }

            // remove right from free list
            block_set_next(prev, block_get_next(right));
            block_set_next(right, 0);

            // add size of right
            block_set_size(left, block_get_size(left) + block_get_size(right) + sizeof(block_t));
        }

        // add size to current block
        block_set_size(left, block_get_size(left) + block_get_size(block) + sizeof(block_t));

        heap_block_insert(heap, prev, left);
    } else {
        if (block_is_free(right)) {
            block_t *prev = heap_block_prev(heap, right);
            block_set_next(prev, block_get_next(right));
            block_set_size(block, block_get_size(block) + block_get_size(right) + sizeof(block_t));
            heap_block_insert(heap, prev, block);
            heap->memory.free_size += sizeof(block_t);
        } else {
            heap_block_insert(heap, heap_block_super(heap), block);
        }
    }
}

bool
heap_is_clean(heap_t *heap)
{
    block_t *super = heap_block_super(heap);
    block_t *first = heap_block_right(heap, super);
    return block_get_next(super) == heap_block_offset(heap, first) &&
           block_get_size(first) == HEAP_FIRST_BLOCK_SIZE;
}

size_t
heap_get_capacity(void)
{
    return HEAP_FIRST_BLOCK_SIZE;
}

size_t
heap_get_free_size(heap_t *heap)
{
    return heap->memory.free_size;
}

// --- private functions
static uint16_t
block_get_size(block_t *block)
{
    return block->size;
}

static void
block_set_size(block_t *block, uint16_t size)
{
    block->size = size;
}

static uint16_t
block_get_next(block_t *block)
{
    return *(uint16_t *)((uint8_t *)block + sizeof(block->size) + block->size);
}

static void
block_set_next(block_t *block, uint16_t next)
{
    *(uint16_t *)((uint8_t *)block + sizeof(block->size) + block->size) = next;
}

static void *
block_get_pointer(block_t *block)
{
    return &block->memory;
}

static uint16_t
block_get_left_next(block_t *block)
{
    return *(&block->size - 1);
}

static bool
block_is_left_free(block_t *block)
{
    return block_get_left_next(block) != 0;
}

static bool
block_is_free(block_t *block)
{
    return block->size != HEAP_GUARD_BLOCK_SIZE && block_get_next(block) != 0;
}

static block_t *
heap_block_at(heap_t *heap, uint16_t offset)
{
    return (block_t *)(&heap->memory.m16[offset / 2]);
}

static block_t *
heap_block_of(heap_t *heap, void *ptr)
{
    uint16_t offset = (uint16_t)((uint8_t *)ptr - heap->memory.m8);
    offset -= sizeof(uint16_t);
    return heap_block_at(heap, offset);
}

static block_t *
heap_block_super(heap_t *heap)
{
    return heap_block_at(heap, HEAP_SUPER_BLOCK_OFFSET);
}

static block_t *
heap_block_next(heap_t *heap, block_t *block)
{
    return heap_block_at(heap, block_get_next(block));
}

static block_t *
heap_block_right(heap_t *heap, block_t *block)
{
    return heap_block_at(heap, heap_block_offset(heap, block) + sizeof(block_t) + block_get_size(block));
}

static block_t *
heap_block_prev(heap_t *heap, block_t *block)
{
    block_t *prev = heap_block_super(heap);
    while (block_get_next(prev) != heap_block_offset(heap, block)) {
        prev = heap_block_next(heap, prev);
    }
    return prev;
}

static bool
heap_is_left_free(heap_t *heap, block_t *block)
{
    return (heap_block_offset(heap, block) != HEAP_FIRST_BLOCK_OFFSET && block_is_left_free(block));
}

static uint16_t
heap_block_offset(heap_t *heap, block_t *block)
{
    return (uint16_t)((uint8_t *)block - heap->memory.m8);
}

static void
heap_block_insert(heap_t *heap, block_t *aprev, block_t *ablock)
{
    block_t *prev = aprev;

    for (block_t *block = heap_block_next(heap, prev);
         block_get_size(block) < block_get_size(ablock);
         block = heap_block_next(heap, block)) {
        prev = block;
    }

    block_set_next(ablock, block_get_next(prev));
    block_set_next(prev, heap_block_offset(heap, ablock));
}
