#ifndef NS_CORE_CRYPTO_HMAC_SHA256_H_
#define NS_CORE_CRYPTO_HMAC_SHA256_H_

#include "core/core-config.h"
#include "mbedtls/md.h"
#include <stdint.h>

typedef struct _crypto_hmac_sha256 crypto_hmac_sha256_t;

enum {
    CRYPTO_HMAC_SHA256_HASH_SIZE = 32, // SHA-256 hash size (bytes)
};

struct _crypto_hmac_sha256 {
    mbedtls_md_context_t context;
};

void
crypto_hmac_sha256_ctor(crypto_hmac_sha256_t *hmac_sha256);

void
crypto_hmac_sha256_dtor(crypto_hmac_sha256_t *hmac_sha256);

void
crypto_hmac_sha256_start(crypto_hmac_sha256_t *hmac_sha256, const uint8_t *key, uint16_t key_length);

void
crypto_hmac_sha256_update(crypto_hmac_sha256_t *hmac_sha256, const uint8_t *buf, uint16_t buf_length);

void
crypto_hmac_sha256_finish(crypto_hmac_sha256_t *hmac_sha256, uint8_t hash[CRYPTO_HMAC_SHA256_HASH_SIZE]);

#endif // NS_CORE_CRYPTO_HMAC_SHA256_H_
