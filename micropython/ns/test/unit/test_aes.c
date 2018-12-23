#include "core/core-config.h"
#include "core/common/instance.h"
#include "core/common/debug.h"
#include "core/crypto/aes_ccm.h"
#include "test_util.h"
#include <string.h>

// verifies test vectors from IEEE 802.15.4-2006 Annex C Section C.2.1
static ns_error_t
test_mac_beacon_frame(void)
{
    ns_error_t error = NS_ERROR_NONE;

    uint8_t key[] = {
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    };

    uint8_t test[] = {0x08, 0xD0, 0x84, 0x21, 0x43, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE,
                      0xAC, 0x02, 0x05, 0x00, 0x00, 0x00, 0x55, 0xCF, 0x00, 0x00, 0x51, 0x52,
                      0x53, 0x54, 0x22, 0x3B, 0xC1, 0xEC, 0x84, 0x1A, 0xB5, 0x53};

    uint8_t encrypted[] = {0x08, 0xD0, 0x84, 0x21, 0x43, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE,
                           0xAC, 0x02, 0x05, 0x00, 0x00, 0x00, 0x55, 0xCF, 0x00, 0x00, 0x51, 0x52,
                           0x53, 0x54, 0x22, 0x3B, 0xC1, 0xEC, 0x84, 0x1A, 0xB5, 0x53};

    uint8_t decrypted[] = {0x08, 0xD0, 0x84, 0x21, 0x43, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE,
                           0xAC, 0x02, 0x05, 0x00, 0x00, 0x00, 0x55, 0xCF, 0x00, 0x00, 0x51, 0x52,
                           0x53, 0x54, 0x22, 0x3B, 0xC1, 0xEC, 0x84, 0x1A, 0xB5, 0x53};

    instance_t *instance = instance_get();

    crypto_aes_ccm_t aes_ccm;
    crypto_aes_ccm_ctor(&aes_ccm);

    uint32_t header_length = sizeof(test) - 8;
    uint32_t payload_length = 0;
    uint8_t tag_length = 8;

    uint8_t nonce[] = {
        0xAC, 0xDE, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x02,
    };

    TEST_VERIFY_OR_EXIT(instance != NULL, "null instance\r\n");

    crypto_aes_ccm_set_key(&aes_ccm, key, sizeof(key));
    crypto_aes_ccm_init(&aes_ccm, header_length, payload_length, tag_length, nonce, sizeof(nonce));
    crypto_aes_ccm_header(&aes_ccm, test, header_length);
    crypto_aes_ccm_finalize(&aes_ccm, test + header_length, &tag_length);

    TEST_VERIFY_OR_EXIT(memcmp(test, encrypted, sizeof(encrypted)) == 0,
                        "test_mac_beacon_frame encrypt failed\r\n");

    crypto_aes_ccm_init(&aes_ccm, header_length, payload_length, tag_length, nonce, sizeof(nonce));
    crypto_aes_ccm_header(&aes_ccm, test, header_length);
    crypto_aes_ccm_finalize(&aes_ccm, test + header_length, &tag_length);

    TEST_VERIFY_OR_EXIT(memcmp(test, decrypted, sizeof(decrypted)) == 0,
                        "test_mac_beacon_frame decrypt failed\r\n");

    crypto_aes_ccm_dtor(&aes_ccm);

exit:
    return error;
}

// verifies test vectors from IEEE 802.15.4-2006 Annex C Section C.2.3
static ns_error_t
test_mac_command_frame(void)
{
    ns_error_t error = NS_ERROR_NONE;

    uint8_t key[] = {
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    };

    uint8_t test[] = {
        0x2B, 0xDC, 0x84, 0x21, 0x43, 0x02, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC,
        0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC, 0x06, 0x05, 0x00,
        0x00, 0x00, 0x01, 0xCE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    uint32_t header_length = 29, payload_length = 1;
    uint8_t  tag_length = 8;

    uint8_t encrypted[] = {
        0x2B, 0xDC, 0x84, 0x21, 0x43, 0x02, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC,
        0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC, 0x06, 0x05, 0x00,
        0x00, 0x00, 0x01, 0xD8, 0x4F, 0xDE, 0x52, 0x90, 0x61, 0xF9, 0xC6, 0xF1,
    };

    uint8_t decrypted[] = {
        0x2B, 0xDC, 0x84, 0x21, 0x43, 0x02, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC,
        0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC, 0x06, 0x05, 0x00,
        0x00, 0x00, 0x01, 0xCE, 0x4F, 0xDE, 0x52, 0x90, 0x61, 0xF9, 0xC6, 0xF1,
    };

    uint8_t nonce[] = {
        0xAC, 0xDE, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x06,
    };

    crypto_aes_ccm_t aes_ccm;
    crypto_aes_ccm_ctor(&aes_ccm);

    crypto_aes_ccm_set_key(&aes_ccm, key, sizeof(key));
    crypto_aes_ccm_init(&aes_ccm, header_length, payload_length, tag_length, nonce, sizeof(nonce));
    crypto_aes_ccm_header(&aes_ccm, test, header_length);
    crypto_aes_ccm_payload(&aes_ccm, test + header_length, test + header_length, payload_length, true);
    crypto_aes_ccm_finalize(&aes_ccm, test + header_length + payload_length, &tag_length);

    TEST_VERIFY_OR_EXIT(memcmp(test, encrypted, sizeof(encrypted)) == 0,
                        "test_mac_command_frame encrypt failed\r\n");

    crypto_aes_ccm_init(&aes_ccm, header_length, payload_length, tag_length, nonce, sizeof(nonce));
    crypto_aes_ccm_header(&aes_ccm, test, header_length);
    crypto_aes_ccm_payload(&aes_ccm, test + header_length, test + header_length, payload_length, false);
    crypto_aes_ccm_finalize(&aes_ccm, test + header_length + payload_length, &tag_length);

    TEST_VERIFY_OR_EXIT(memcmp(test, decrypted, sizeof(decrypted)) == 0,
                        "test_mac_command_frame decrypt failed\r\n");

    crypto_aes_ccm_dtor(&aes_ccm);

exit:
    return error;
}

ns_error_t
test_crypto_aes(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("---------------------------- TEST CRYPTO AES\r\n");

    VERIFY_OR_EXIT((error = test_mac_beacon_frame()) == NS_ERROR_NONE);
    VERIFY_OR_EXIT((error = test_mac_command_frame()) == NS_ERROR_NONE);

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
