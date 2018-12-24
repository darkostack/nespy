#include "core/crypto/hmac_sha256.h"

void
crypto_hmac_sha256_ctor(crypto_hmac_sha256_t *hmac_sha256)
{
    const mbedtls_md_info_t *md_info = NULL;
    mbedtls_md_init(&hmac_sha256->context);
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_setup(&hmac_sha256->context, md_info, 1);
}

void
crypto_hmac_sha256_dtor(crypto_hmac_sha256_t *hmac_sha256)
{
    mbedtls_md_free(&hmac_sha256->context);
}

void
crypto_hmac_sha256_start(crypto_hmac_sha256_t *hmac_sha256, const uint8_t *key, uint16_t key_length)
{
    mbedtls_md_hmac_starts(&hmac_sha256->context, key, key_length);
}

void
crypto_hmac_sha256_update(crypto_hmac_sha256_t *hmac_sha256, const uint8_t *buf, uint16_t buf_length)
{
    mbedtls_md_hmac_update(&hmac_sha256->context, buf, buf_length);
}

void
crypto_hmac_sha256_finish(crypto_hmac_sha256_t *hmac_sha256, uint8_t hash[CRYPTO_HMAC_SHA256_HASH_SIZE])
{
    mbedtls_md_hmac_finish(&hmac_sha256->context, hash);
}
