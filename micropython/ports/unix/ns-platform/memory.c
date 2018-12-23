#include "ns/platform/memory.h"

void *
ns_plat_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void
ns_plat_free(void *ptr)
{
    free(ptr);
}
