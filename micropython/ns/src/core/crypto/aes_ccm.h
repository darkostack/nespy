#ifndef NS_CORE_CRYPTO_AES_CCM_H_
#define NS_CORE_CRYPTO_AES_CCM_H_

#include "core/core-config.h"
#include "ns/error.h"
#include "core/crypto/aes_ecb.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct _crypto_aes_ccm crypto_aes_ccm_t;

enum {
    CRYPTO_AES_CCM_TAG_LENGTH_MIN = 4,
};

struct _crypto_aes_ccm {
    crypto_aes_ecb_t ecb;
    uint8_t block[CRYPTO_AES_ECB_BLOCK_SIZE];
    uint8_t ctr[CRYPTO_AES_ECB_BLOCK_SIZE];
    uint8_t ctr_pad[CRYPTO_AES_ECB_BLOCK_SIZE];
    uint8_t nonce_length;
    uint32_t header_length;
    uint32_t header_cur;
    uint32_t plain_text_length;
    uint32_t plain_text_cur;
    uint16_t block_length;
    uint16_t ctr_length;
    uint8_t tag_length;
};

void
crypto_aes_ccm_ctor(crypto_aes_ccm_t *aes_ccm);

void
crypto_aes_ccm_dtor(crypto_aes_ccm_t *aes_ccm);

ns_error_t
crypto_aes_ccm_set_key(crypto_aes_ccm_t *aes_ccm, const uint8_t *key, uint16_t key_length);

ns_error_t
crypto_aes_ccm_init(crypto_aes_ccm_t *aes_ccm,
                    uint32_t header_length,
                    uint32_t plain_text_length,
                    uint8_t tag_length,
                    const void *nonce,
                    uint8_t nonce_length);

void
crypto_aes_ccm_header(crypto_aes_ccm_t *aes_ccm,
                      const void *header,
                      uint32_t header_length);

void
crypto_aes_ccm_payload(crypto_aes_ccm_t *aes_ccm,
                       void *plain_text,
                       void *cipher_text,
                       uint32_t length,
                       bool encrypt);

void
crypto_aes_ccm_finalize(crypto_aes_ccm_t *aes_ccm, void *tag, uint8_t *tag_length);

#endif // NS_CORE_CRYPTO_AES_CCM_H_
