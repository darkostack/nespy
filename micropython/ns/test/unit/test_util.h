#ifndef NS_TEST_UNIT_TEST_UTIL_H_
#define NS_TEST_UNIT_TEST_UTIL_H_

#define TEST_VERIFY_OR_EXIT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAILED %s:%d - %s", __FUNCTION__, __LINE__, msg); \
            error = NS_ERROR_FAILED; \
            goto exit; \
        } \
    } while (false)

#endif // NS_TEST_UNIT_TEST_UTIL_H_
