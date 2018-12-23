#include "core/crypto/aes_ecb.h"

void
crypto_aes_ecb_ctor(crypto_aes_ecb_t *aes_ecb)
{
    mbedtls_aes_init(&aes_ecb->context);
}

void
crypto_aes_ecb_dtor(crypto_aes_ecb_t *aes_ecb)
{
    mbedtls_aes_free(&aes_ecb->context);
}

void
crypto_aes_ecb_set_key(crypto_aes_ecb_t *aes_ecb, const uint8_t *key, uint16_t key_length)
{
    mbedtls_aes_setkey_enc(&aes_ecb->context, key, key_length);
}

void
crypto_aes_ecb_encrypt(crypto_aes_ecb_t *aes_ecb,
                       const uint8_t input[CRYPTO_AES_ECB_BLOCK_SIZE],
                       uint8_t output[CRYPTO_AES_ECB_BLOCK_SIZE])
{
    mbedtls_aes_crypt_ecb(&aes_ecb->context, MBEDTLS_AES_ENCRYPT, input, output);
}
