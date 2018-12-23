#ifndef NS_PLATFORM_MEMORY_H_
#define NS_PLATFORM_MEMORY_H_

#include <stdlib.h>

void *
ns_plat_calloc(size_t num, size_t size);

void
ns_plat_free(void *ptr);

#endif // NS_PLATFORM_MEMORY_H_
