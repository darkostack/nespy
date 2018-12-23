#ifndef NS_CORE_CRYPTO_SHA256_H_
#define NS_CORE_CRYPTO_SHA256_H_

#include "core/core-config.h"
#include "mbedtls/sha256.h"
#include <stdint.h>

typedef struct _crypto_sha256 crypto_sha256_t;

enum {
    CRYPTO_SHA256_HASH_SIZE = 32,
};

struct _crypto_sha256 {
    mbedtls_sha256_context context;
};

void
crypto_sha256_ctor(crypto_sha256_t *sha256);

void
crypto_sha256_dtor(crypto_sha256_t *sha256);

void
crypto_sha256_start(crypto_sha256_t *sha256);

void
crypto_sha256_update(crypto_sha256_t *sha256, const uint8_t *buf, uint16_t buf_length);

void
crypto_sha256_finish(crypto_sha256_t *sha256, uint8_t hash[CRYPTO_SHA256_HASH_SIZE]);

#endif // NS_CORE_CRYPTO_SHA256_H_
