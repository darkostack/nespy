#include "core/crypto/mbedtls.h"
#include "core/common/instance.h"
#include "mbedtls/platform.h"

static void *
ns_heap_calloc(size_t count, size_t size)
{
    return heap_calloc(instance_get_heap(instance_get()), count, size);
}

static void
ns_heap_free(void *pointer)
{
    heap_free(instance_get_heap(instance_get()), pointer);
}

void
mbedtls_ctor(void)
{
    mbedtls_platform_set_calloc_free(ns_heap_calloc, ns_heap_free);
}
