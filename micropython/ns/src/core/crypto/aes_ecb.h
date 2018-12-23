#ifndef NS_CORE_CRYPTO_AES_ECB_H_
#define NS_CORE_CRYPTO_AES_ECB_H_

#include "core/core-config.h"
#include "mbedtls/aes.h"

typedef struct _crypto_aes_ecb crypto_aes_ecb_t;

enum {
    CRYPTO_AES_ECB_BLOCK_SIZE = 16, // AES-128 block size (bytes)
};

struct _crypto_aes_ecb {
    mbedtls_aes_context context;
};

void
crypto_aes_ecb_ctor(crypto_aes_ecb_t *aes_ecb);

void
crypto_aes_ecb_dtor(crypto_aes_ecb_t *aes_ecb);

void
crypto_aes_ecb_set_key(crypto_aes_ecb_t *aes_ecb, const uint8_t *key, uint16_t key_length);

void
crypto_aes_ecb_encrypt(crypto_aes_ecb_t *aes_ecb,
                       const uint8_t input[CRYPTO_AES_ECB_BLOCK_SIZE],
                       uint8_t output[CRYPTO_AES_ECB_BLOCK_SIZE]);

#endif // NS_CORE_CRYPTO_AES_ECB_H_
