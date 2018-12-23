#include "core/crypto/pbkdf2_cmac.h"
#include "core/common/debug.h"
#include "mbedtls/cmac.h"
#include <string.h>

#if NS_ENABLE_COMMISSIONER && NS_FTD
void
ns_pbkdf2_cmac(const uint8_t *password,
               uint16_t password_len,
               const uint8_t *salt,
               uint16_t salt_len,
               uint32_t iteration_counter,
               uint16_t akey_len,
               uint8_t *akey)
{
    const size_t block_size = MBEDTLS_CIPHER_BLKSIZE_MAX;
    uint8_t prf_input[NS_PBKDF2_SALT_MAX_LEN + 4]; // salt || INT(), for U1 calculation
    long prf_one[block_size / sizeof(long)];
    long prf_two[block_size / sizeof(long)];
    long key_block[block_size / sizeof(long)];
    uint32_t block_counter = 0;
    uint8_t *key = akey;
    uint16_t key_len = akey_len;
    uint16_t use_len = 0;

    memcpy(prf_input, salt, salt_len);
    assert(iteration_counter % 2 == 0);
    iteration_counter /= 2;

    while (key_len) {
        ++block_counter;
        prf_input[salt_len + 0] = (uint8_t)(block_counter >> 24);
        prf_input[salt_len + 1] = (uint8_t)(block_counter >> 16);
        prf_input[salt_len + 2] = (uint8_t)(block_counter >> 8);
        prf_input[salt_len + 3] = (uint8_t)(block_counter);

        // calculate U1
        mbedtls_aes_cmac_prf_128(password, password_len, prf_input, salt_len + 4, (uint8_t *)key_block);

        // calculate U2
        mbedtls_aes_cmac_prf_128(password, password_len, (const uint8_t *)key_block, block_size,
                                 (uint8_t *)prf_one);

        for (uint32_t j = 0; j < block_size / sizeof(long); ++j) {
            key_block[j] ^= prf_one[j];
        }

        for (uint32_t i = 1; i < iteration_counter; ++i) {
            // calculate U{2 * i - 1}
            mbedtls_aes_cmac_prf_128(password, password_len, (const uint8_t *)prf_one, block_size,
                                     (uint8_t *)prf_two);
            // calculate U{2 * i}
            mbedtls_aes_cmac_prf_128(password, password_len, (const uint8_t *)prf_two, block_size,
                                     (uint8_t *)prf_one);
            for (uint32_t j = 0; j < block_size / sizeof(long); ++j) {
                key_block[j] ^= prf_one[j] ^ prf_two[j];
            }
        }

        use_len = (key_len < block_size) ? key_len : block_size;
        memcpy(key, key_block, use_len);
        key += use_len;
        key_len -= use_len;
    }
}
#endif // #if NS_ENABLE_COMMISSIONER && NS_FTD
