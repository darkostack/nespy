#include "core/crypto/aes_ccm.h"
#include "core/common/code_utils.h"
#include "core/common/debug.h"

ns_error_t
crypto_aes_ccm_set_key(crypto_aes_ccm_t *aes_ccm, const uint8_t *key, uint16_t key_length)
{
    crypto_aes_ecb_set_key(&aes_ccm->ecb, key, 8 * key_length);
    return NS_ERROR_NONE;
}

ns_error_t
crypto_aes_ccm_init(crypto_aes_ccm_t *aes_ccm,
                    uint32_t header_length,
                    uint32_t plain_text_length,
                    uint8_t tag_length,
                    const void *nonce,
                    uint8_t nonce_length)
{
    const uint8_t *nonce_bytes = (const uint8_t *)nonce;
    ns_error_t error = NS_ERROR_NONE;
    uint8_t block_length = 0;
    uint32_t len;
    uint8_t L;
    uint8_t i;

    // tag_length must be even
    tag_length &= ~1;

    if (tag_length > sizeof(aes_ccm->block)) {
        tag_length = sizeof(aes_ccm->block);
    } else if (tag_length < CRYPTO_AES_CCM_TAG_LENGTH_MIN) {
        EXIT_NOW(error = NS_ERROR_INVALID_ARGS);
    }

    L = 0;

    for (len = plain_text_length; len; len >>= 8) {
        L++;
    }

    if (L <= 1) {
        L = 2;
    }

    if (nonce_length > 13) {
        nonce_length = 13;
    }

    // increase L to match nonce len
    if (L < (15 - nonce_length)) {
        L = 15 - nonce_length;
    }

    // decrease nonce length to match L
    if (nonce_length > (15 - L)) {
        nonce_length = 15 - L;
    }

    // setup initial block

    // write flags
    aes_ccm->block[0] = ((uint8_t)((header_length != 0) << 6) |
                         (uint8_t)(((tag_length - 2) >> 1) << 3) |
                         (uint8_t)(L - 1));

    // write nonce
    for (i = 0; i < nonce_length; i++) {
        aes_ccm->block[1 + i] = nonce_bytes[i];
    }

    // write len
    len = plain_text_length;

    for (i = sizeof(aes_ccm->block) - 1; i > nonce_length; i--) {
        aes_ccm->block[i] = len & 0xff;
        len >>= 8;
    }

    // encrypt initial block
    crypto_aes_ecb_encrypt(&aes_ccm->ecb, aes_ccm->block, aes_ccm->block);

    // process header
    if (header_length > 0) {
        // process length
        if (header_length < (65536U - 256U)) {
            aes_ccm->block[block_length++] ^= header_length >> 8;
            aes_ccm->block[block_length++] ^= header_length >> 0;
        } else {
            aes_ccm->block[block_length++] ^= 0xff;
            aes_ccm->block[block_length++] ^= 0xfe;
            aes_ccm->block[block_length++] ^= header_length >> 24;
            aes_ccm->block[block_length++] ^= header_length >> 16;
            aes_ccm->block[block_length++] ^= header_length >> 8;
            aes_ccm->block[block_length++] ^= header_length >> 0;
        }
    }

    // init counter
    aes_ccm->ctr[0] = L - 1;

    for (i = 0; i < nonce_length; i++) {
        aes_ccm->ctr[1 + i] = nonce_bytes[i];
    }

    for (i = i + 1; i < sizeof(aes_ccm->ctr); i++) {
        aes_ccm->ctr[i] = 0;
    }

    aes_ccm->nonce_length = nonce_length;
    aes_ccm->header_length = header_length;
    aes_ccm->header_cur = 0;
    aes_ccm->plain_text_length = plain_text_length;
    aes_ccm->plain_text_cur = 0;
    aes_ccm->block_length = block_length;
    aes_ccm->ctr_length = sizeof(aes_ccm->ctr_pad);
    aes_ccm->tag_length = tag_length;

exit:
    return error;
}

void
crypto_aes_ccm_header(crypto_aes_ccm_t *aes_ccm,
                      const void *header,
                      uint32_t header_length)
{
    const uint8_t *header_bytes = (const uint8_t *)(header);

    assert(aes_ccm->header_cur + header_length <= aes_ccm->header_length);

    // process header
    for (unsigned i = 0; i < header_length; i++) {
        if (aes_ccm->block_length == sizeof(aes_ccm->block)) {
            crypto_aes_ecb_encrypt(&aes_ccm->ecb, aes_ccm->block, aes_ccm->block);
            aes_ccm->block_length = 0;
        }
        aes_ccm->block[aes_ccm->block_length++] ^= header_bytes[i];
    }

    aes_ccm->header_cur += header_length;

    if (aes_ccm->header_cur == aes_ccm->header_length) {
        // process remainder
        if (aes_ccm->block_length != 0) {
            crypto_aes_ecb_encrypt(&aes_ccm->ecb, aes_ccm->block, aes_ccm->block);
        }
        aes_ccm->block_length = 0;
    }
}

void
crypto_aes_ccm_payload(crypto_aes_ccm_t *aes_ccm,
                       void *plain_text,
                       void *cipher_text,
                       uint32_t length,
                       bool encrypt)
{
    uint8_t *plain_text_bytes = (uint8_t *)(plain_text);
    uint8_t *cipher_text_bytes = (uint8_t *)(cipher_text);
    uint8_t byte;

    assert(aes_ccm->plain_text_cur + length <= aes_ccm->plain_text_length);

    for (unsigned i = 0; i < length; i++) {
        if (aes_ccm->ctr_length == 16) {
            for (int j = sizeof(aes_ccm->ctr) - 1; j > aes_ccm->nonce_length; j--) {
                if (++aes_ccm->ctr[j]) {
                    break;
                }
            }
            crypto_aes_ecb_encrypt(&aes_ccm->ecb, aes_ccm->ctr, aes_ccm->ctr_pad);
            aes_ccm->ctr_length = 0;
        }
        if (encrypt) {
            byte = plain_text_bytes[i];
            cipher_text_bytes[i] = byte ^ aes_ccm->ctr_pad[aes_ccm->ctr_length++];
        } else {
            byte = cipher_text_bytes[i] ^ aes_ccm->ctr_pad[aes_ccm->ctr_length++];
            plain_text_bytes[i] = byte;
        }
        if (aes_ccm->block_length == sizeof(aes_ccm->block)) {
            crypto_aes_ecb_encrypt(&aes_ccm->ecb, aes_ccm->block, aes_ccm->block);
            aes_ccm->block_length = 0;
        }
        aes_ccm->block[aes_ccm->block_length++] ^= byte;
    }

    aes_ccm->plain_text_cur += length;

    if (aes_ccm->plain_text_cur >= aes_ccm->plain_text_length) {
        if (aes_ccm->block_length != 0) {
            crypto_aes_ecb_encrypt(&aes_ccm->ecb, aes_ccm->block, aes_ccm->block);
        }
        // reset counter
        for (uint8_t i = aes_ccm->nonce_length + 1; i < sizeof(aes_ccm->ctr); i++) {
            aes_ccm->ctr[i] = 0;
        }
    }
}

void
crypto_aes_ccm_finalize(crypto_aes_ccm_t *aes_ccm, void *tag, uint8_t *tag_length)
{
    uint8_t *tag_bytes = (uint8_t *)tag;

    assert(aes_ccm->plain_text_cur == aes_ccm->plain_text_length);

    if (aes_ccm->tag_length > 0) {
        crypto_aes_ecb_encrypt(&aes_ccm->ecb, aes_ccm->ctr, aes_ccm->ctr_pad);
        for (int i = 0; i < aes_ccm->tag_length; i++) {
            tag_bytes[i] = aes_ccm->block[i] ^ aes_ccm->ctr_pad[i];
        }
    }
    
    if (tag_length) {
        *tag_length = aes_ccm->tag_length;
    }
}
