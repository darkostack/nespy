#ifndef NS_CRYPTO_H_
#define NS_CRYPTO_H_

#include "ns/error.h"
#include <stdbool.h>
#include <stdint.h>

#define NS_CRYPTO_HMAC_SHA_HASH_SIZE 32 // length of HMAC SHA (in bytes)

void
ns_crypto_hmac_sha256(const uint8_t *key,
                      uint16_t key_length,
                      const uint8_t *buf,
                      uint16_t buf_length,
                      uint8_t *hash);

void
ns_crypto_aes_ccm(const uint8_t *key,
                  uint16_t key_length,
                  uint8_t tag_length,
                  const void *nonce,
                  uint8_t nonce_length,
                  const void *header,
                  uint32_t header_length,
                  void *plain_text,
                  void *cipher_text,
                  uint32_t length,
                  bool encrypt,
                  void *tag);

ns_error_t
ns_crypto_ecdsa_sign(uint8_t *output,
                     uint16_t *output_length,
                     const uint8_t *input_hash,
                     uint16_t input_hash_length,
                     const uint8_t *private_key,
                     uint16_t private_key_length);

#endif // NS_CRYPTO_H_
