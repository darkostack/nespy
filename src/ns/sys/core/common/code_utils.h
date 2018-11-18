#ifndef NS_CORE_COMMON_CODE_UTILS_H
#define NS_CORE_COMMON_CODE_UTILS_H

#define NS_ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#define SUCCESS_OR_EXIT(status) \
    do {                        \
        if ((status) != 0) {    \
            goto exit;          \
        }                       \
    } while (false)

#define VERIFY_OR_EXIT(condition, ...) \
    do {                               \
        if (!(condition)) {            \
            __VA_ARGS__;               \
            goto exit;                 \
        }                              \
    } while (false)

#define EXIT_NOW(...) \
    do {              \
        __VA_ARGS__;  \
        goto exit;    \
    } while (false)

#endif // NS_CORE_COMMON_CODE_UTILS_H
