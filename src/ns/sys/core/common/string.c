#include "ns/sys/core/common/string.h"

// --- private functions declarations
static ns_error_t
str_write(string_t *str, const char *format, va_list args);

// --- string functions
void
string_clear(string_t *str)
{
    str->buffer[0] = 0;
    str->length = 0;
}

uint16_t
string_get_length(string_t *str)
{
    return str->length;
}

const char *
string_as_c_string(string_t *str)
{
    return (const char *)str->buffer;
}

ns_error_t
string_set(string_t *str, const char *format, ...)
{
    va_list args;
    ns_error_t error;
    va_start(args, format);
    str->length = 0;
    error = str_write(str, format, args);
    va_end(args);
    return error;
}

ns_error_t
string_append(string_t *str, const char *format, ...)
{
    va_list args;
    ns_error_t error;
    va_start(args, format);
    error = str_write(str, format, args);
    va_end(args);
    return error;
}

void
string_copy(string_t *str_dest, string_t *str_source, uint16_t length)
{
    memcpy(str_dest->buffer, str_source->buffer, length);
}

// --- private functions
static ns_error_t
str_write(string_t *str, const char *format, va_list args)
{
    ns_error_t error = NS_ERROR_NONE;
    int len;
    len = vsnprintf(str->buffer + str->length, str->bufsize - str->length, format, args);
    if (len < 0) {
        str->length = 0;
        str->buffer[0] = 0;
        error = NS_ERROR_INVALID_ARGS;
    } else if (len >= str->bufsize - str->length) {
        str->length = str->bufsize - 1;
        error = NS_ERROR_NO_BUFS;
    } else {
        str->length += (uint16_t)len;
    }
    return error;
}
