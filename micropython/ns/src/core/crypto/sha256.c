#include "core/crypto/sha256.h"

void
crypto_sha256_ctor(crypto_sha256_t *sha256)
{
    mbedtls_sha256_init(&sha256->context);
}

void
crypto_sha256_dtor(crypto_sha256_t *sha256)
{
    mbedtls_sha256_free(&sha256->context);
}

void
crypto_sha256_start(crypto_sha256_t *sha256)
{
    mbedtls_sha256_starts_ret(&sha256->context, 0);
}

void
crypto_sha256_update(crypto_sha256_t *sha256, const uint8_t *buf, uint16_t buf_length)
{
    mbedtls_sha256_update_ret(&sha256->context, buf, buf_length);
}

void
crypto_sha256_finish(crypto_sha256_t *sha256, uint8_t hash[CRYPTO_SHA256_HASH_SIZE])
{
    mbedtls_sha256_finish_ret(&sha256->context, hash);
}
