#include "core/core-config.h"
#include "core/common/instance.h"
#include "core/common/debug.h"
#include "core/crypto/hmac_sha256.h"
#include "test_util.h"
#include <string.h>

typedef struct _hmac_sha256_test hmac_sha256_test_t;
struct _hmac_sha256_test {
    const char *key;
    const char *data;
    uint8_t hash[CRYPTO_HMAC_SHA256_HASH_SIZE];
};

static hmac_sha256_test_t s_tests[] = {
    {
        "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
        "Hi There",
        {
            0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
            0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7, 0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
        },
    },
    {
        NULL,
        NULL,
        {},
    },
};

ns_error_t
test_hmac_sha256(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("-------------------- TEST CRYPTO HMAC SHA256\r\n");

    instance_t *instance = instance_get();

    TEST_VERIFY_OR_EXIT(instance != NULL, "null instance\r\n");

    crypto_hmac_sha256_t hmac;
    uint8_t hash[CRYPTO_HMAC_SHA256_HASH_SIZE];

    crypto_hmac_sha256_ctor(&hmac);

    for (int i = 0; s_tests[i].key != NULL; i++) {
        crypto_hmac_sha256_start(&hmac, (const uint8_t *)s_tests[i].key, (uint16_t)strlen(s_tests[i].key));
        crypto_hmac_sha256_update(&hmac, (const uint8_t *)s_tests[i].data, (uint16_t)strlen(s_tests[i].data));
        crypto_hmac_sha256_finish(&hmac, hash);
        TEST_VERIFY_OR_EXIT(memcmp(hash, s_tests[i].hash, sizeof(s_tests[i].hash)) == 0,
                            "test HMAC-SHA-256 failed\r\n");
    }

    crypto_hmac_sha256_dtor(&hmac);

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
