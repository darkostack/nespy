#ifndef NS_MODULES_NSTD_H_
#define NS_MODULES_NSTD_H_

int ns_tolower(int chr);
int ns_strncmp(const char *s1, const char *s2, size_t n);
int ns_strcmp(const char *s1, const char *s2);
size_t ns_strlen(const char *s);
void ns_memcpy(void *dest, const void *src, size_t n);
size_t ns_strncpy(char *dest, const char *src, size_t size);
size_t ns_strncat(char *dest, const char *src, size_t size);
int ns_strncasecmp(const char *s1, const char *s2, size_t n);
int ns_strcasecmp(const char *s1, const char *s2);

#endif // NS_MODULES_NSTD_H_

