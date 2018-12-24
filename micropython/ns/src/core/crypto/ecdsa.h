#ifndef NS_CORE_CRYPTO_ECDSA_H_
#define NS_CORE_CRYPTO_ECDSA_H_

#include "core/core-config.h"
#include "ns/error.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#if NS_ENABLE_ECDSA
typedef struct _crypto_ecdsa crypto_ecdsa_t;
struct _crypto_ecdsa {
    ns_error_t (*sign)(uint8_t *output,
                       uint16_t *output_length,
                       const uint8_t *input_hash,
                       uint16_t input_hash_length,
                       const uint8_t *private_key,
                       uint16_t private_key_length);
};

void
crypto_ecdsa_ctor(crypto_ecdsa_t *ecdsa);
#endif // NS_ENABLE_ECDSA

#endif // NS_CORE_CRYPTO_ECDSA_H_
