#ifndef NS_CORE_COMMON_CODE_UTILS_H
#define NS_CORE_COMMON_CODE_UTILS_H

#define NS_ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#define NS_DEFINE_ALIGNED_VAR(name, size, align_type) \
    align_type name[(((size) + (sizeof(align_type) - 1)) / sizeof(align_type))]

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

#define EXPECT(condition)   \
    do {                    \
        if (!(condition)) { \
            goto exit;      \
        }                   \
    } while (0)

#define EXPECT_ACTION(condition, action) \
    do {                                 \
        if (!(condition)) {              \
            action;                      \
            goto exit;                   \
        }                                \
    } while (0)

#endif // NS_CORE_COMMON_CODE_UTILS_H
