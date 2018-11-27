#include "ns/include/error.h"
#include "ns/sys/core/utils/heap.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>

ns_error_t
test_heap_allocate_single(void)
{
    ns_error_t error = NS_ERROR_NONE;
    heap_t heap;

    printf("------------------ TEST HEAP ALLOCATE SINGLE\r\n");

    heap_make_new(&heap);

    TEST_VERIFY_OR_EXIT(heap_is_clean(&heap),
                        "heap is not clean as expected.\r\n");

    size_t total_size = heap_get_free_size(&heap);

    {
        void *p = heap_calloc(&heap, 1, 0);

        TEST_VERIFY_OR_EXIT(p == NULL && total_size == heap_get_free_size(&heap),
                            "test heap allocate single allocate 1 x 0 byte failed.\r\n");

        heap_free(&heap, p);

        p = heap_calloc(&heap, 0, 1);

        TEST_VERIFY_OR_EXIT(p == NULL && total_size == heap_get_free_size(&heap),
                            "test heap allocate single allocate 0 x 1 byte failed.\r\n");
    }

    for (size_t size = 1; size <= heap_get_capacity(&heap); ++size) {
        void *p = heap_calloc(&heap, 1, size);
        TEST_VERIFY_OR_EXIT(p != NULL && !heap_is_clean(&heap) && heap_get_free_size(&heap) + size <= total_size,
                            "allocating failed.\r\n");
        memset(p, 0xff, size);
        heap_free(&heap, p);
        TEST_VERIFY_OR_EXIT(heap_is_clean(&heap) && heap_get_free_size(&heap) == total_size,
                            "freeing failed.\r\n");
    }

    // verify data of the allocates heap
    size_t allocate_size = 100;

    void *d = heap_calloc(&heap, 1, allocate_size);
    memset(d, 0xff, allocate_size);

    uint8_t *data = (uint8_t *)d;

    for (int i = 0; i < allocate_size; i++) {
        TEST_VERIFY_OR_EXIT(*data == 0xff,
                            "allocated data is not as expected.\r\n");
        data++;
    }

    heap_free(&heap, d);
    TEST_VERIFY_OR_EXIT(heap_is_clean(&heap) && heap_get_free_size(&heap) == total_size,
                        "freeing failed.\r\n");

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }

    return error;
}
