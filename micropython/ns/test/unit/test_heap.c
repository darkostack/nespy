#include "ns/error.h"
#include "ns/platform/random.h"
#include "core/common/instance.h"
#include "test_util.h"
#include <string.h>
#include <stdio.h>

ns_error_t
test_heap_allocate_single(void *instance)
{
    ns_error_t error = NS_ERROR_NONE;
    heap_t *heap = &((instance_t *)instance)->heap;

    printf("------------------ TEST HEAP ALLOCATE SINGLE\r\n");

    TEST_VERIFY_OR_EXIT(heap_is_clean(heap),
                        "heap is not clean as expected.\r\n");

    size_t total_size = heap_get_free_size(heap);

    {
        void *p = heap_calloc(heap, 1, 0);

        TEST_VERIFY_OR_EXIT(p == NULL &&
                            total_size == heap_get_free_size(heap),
                            "test heap allocate single allocate 1 x 0 byte failed.\r\n");

        heap_free(heap, p);

        p = heap_calloc(heap, 0, 1);

        TEST_VERIFY_OR_EXIT(p == NULL &&
                            total_size == heap_get_free_size(heap),
                            "test heap allocate single allocate 0 x 1 byte failed.\r\n");

        heap_free(heap, p);
    }

    for (size_t size = 1; size <= heap_get_capacity(); ++size) {
        void *p = heap_calloc(heap, 1, size);
        //printf("test heap allocate single allocating %lu bytes...\r\n", size);
        TEST_VERIFY_OR_EXIT(p != NULL &&
                            !heap_is_clean(heap) &&
                            heap_get_free_size(heap) + size <= total_size,
                            "allocating failed.\r\n");
        memset(p, 0xff, size);
        heap_free(heap, p);
        TEST_VERIFY_OR_EXIT(heap_is_clean(heap) &&
                            heap_get_free_size(heap) == total_size,
                            "freeing failed.\r\n");
    }

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }

    return error;
}

typedef struct _node node_t;
struct _node {
    node_t *next;
    size_t size;
};

node_t head;

static ns_error_t
test_heap_allocate_randomly(void *instance, size_t size_limit)
{
    ns_error_t error = NS_ERROR_NONE;
    heap_t *heap = &((instance_t *)instance)->heap;
    node_t *last = &head;
    size_t nnodes = 0;

    TEST_VERIFY_OR_EXIT(heap_is_clean(heap),
                        "heap is not clean as expected.\r\n");

    size_t total_size = heap_get_free_size(heap);

    while (1) {
        size_t size = (size_t)ns_plat_random_get() % size_limit + sizeof(node_t);
        last->next = (node_t *)heap_calloc(heap, 1, size);
        if (last->next == NULL) {
            break;
        }
        //printf("test heap allocate randomly allocating %lu bytes...\r\n", size);
        last = last->next;
        last->size = size;
        nnodes++;

        // 50 % probability to randomly free a node
        size_t free_index = (size_t)(ns_plat_random_get() % (nnodes * 2));

        if (free_index > nnodes) {
            free_index /= 2;
            node_t *prev = &head;
            while (free_index--) {
                prev = prev->next;
            }
            node_t *curr = prev->next;
            //printf("test heap allocate randomly freeing %lu bytes...\r\n", last->size);
            prev->next = curr->next;
            heap_free(heap, curr);
            if (last == curr) {
                last = prev;
            }
            --nnodes;
        }
    }

    last = head.next;

    while (last) {
        node_t *next = last->next;
        //printf("test heap allocate randomly freeing %lu bytes...\r\n", last->size);
        heap_free(heap, last);
        last = next;
        --nnodes;
    }

    TEST_VERIFY_OR_EXIT(heap_is_clean(heap) &&
                        heap_get_free_size(heap) == total_size &&
                        nnodes == 0,
                        "freeing failed.\r\n");

exit:
    return error;
}

ns_error_t
test_heap_allocate_multiple(void *instance)
{
    ns_error_t error = NS_ERROR_NONE;
    printf("---------------- TEST HEAP ALLOCATE MULTIPLE\r\n");
    for (int i = 0; i < 10; i++) {
        size_t size_limit = (1 << i);
        TEST_VERIFY_OR_EXIT(test_heap_allocate_randomly(instance, size_limit) == NS_ERROR_NONE,
                            "heap allocate randomly failed\r\n");
    }
exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
