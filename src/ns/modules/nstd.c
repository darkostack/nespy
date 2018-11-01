#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int ns_tolower(int chr)
{
    return (chr >= 'A' && chr <= 'Z') ? (chr + 32) : (chr);
}

int ns_strncmp(const char *s1, const char *s2, size_t n)
{
    for ( ; n--; ++s1, ++s2) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
    }

    return 0;
}

int ns_strcmp(const char *s1, const char *s2)
{
    while ((*s1 != '\0' && *s2 != '\0') && *s1 == *s2) {
        s1++;
        s2++;
    }

    if (*s1 == *s2) {
        return 0; // strings are identical
    } else {
        return *s1 - *s2;
    }
}

size_t ns_strlen(const char *s)
{
    size_t ret;

    for (ret = 0; s[ret] != 0; ret++) {
        // empty loop
    }

    return ret;
}

void ns_memcpy(void *dest, const void *src, size_t n)
{
    const char *csrc = (const char *)src;
    char *cdest = (char *)dest;

    for (int i = 0; i < n; i++) {
        cdest[i] = csrc[i];
    }
}

size_t ns_strncpy(char *dest, const char *src, size_t size)
{
    const size_t slen = ns_strlen(src);

    if (size != 0) {
        size--;
        if (slen < size) {
            size = slen;
        }
        if (size != 0) {
            ns_memcpy(dest, src, size);
        }
        dest[size] = 0;
    }

    return slen;
}

size_t ns_strncat(char *dest, const char *src, size_t size)
{
    size_t len = ns_strlen(dest);

    if (len < size - 1)
    {
        return (len + ns_strncpy(dest + len, src, size - len));
    }

    return len + ns_strlen(src);
}

int ns_strncasecmp(const char *s1, const char *s2, size_t n)
{
    return ns_strncmp(s1, s2, n);
}

int ns_strcasecmp(const char *s1, const char *s2)
{
  return ns_strcmp(s1, s2);
}

void ns_log(const char *format, ...)
{
    char log_string[512];
    int chars_written;
    va_list args;
    va_start(args, format);
    chars_written = vsnprintf(&log_string[0], sizeof(log_string), format, args);
    va_end(args);
    printf("%s\r\n", log_string);
}
