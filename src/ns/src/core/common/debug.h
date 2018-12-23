#ifndef NS_CORE_COMMON_DEBUG_H_
#define NS_CORE_COMMON_DEBUG_H_

#include "core/core-config.h"
#include <ctype.h>
#include <stdio.h>

#if NS_CONFIG_PLATFORM_ASSERT_MANAGEMENT
#include "ns/platform/misc.h"

#ifndef FILE_NAME
#define FILE_NAME __FILE__
#endif

#define assert(cond)                                  \
    do {                                              \
        if (!(cond)) {                                \
            ns_plat_assert_fail(FILE_NAME, __LINE__); \
        }                                             \
    } while (0)

#else
#include <assert.h>
#endif

#endif // NS_CORE_COMMON_DEBUG_H_
