#ifndef NS_CORE_COMMON_STRING_H_
#define NS_CORE_COMMON_STRING_H_

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ns/include/error.h"
#include "ns/sys/core/core-config.h"

#define STRING(name, size)                      \
    static char name##_string_buffer[size];     \
    static string_t name##_string = {           \
        .length = 0,                            \
        .bufsize = size,                        \
        .buffer = name##_string_buffer,         \
    }

typedef struct _string string_t;

struct _string {
    uint16_t length;
    uint16_t bufsize;
    char *buffer;
};

void
string_clear(string_t *str);

uint16_t
string_get_length(string_t *str);

const char *
string_as_c_string(string_t *str);

ns_error_t
string_set(string_t *str, const char *format, ...);

ns_error_t
string_append(string_t *str, const char *format, ...);

#endif // NS_CORE_COMMON_STRING_H_
