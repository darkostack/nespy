#ifndef NS_CORE_CRYPTO_PBKDF2_CMAC_H_
#define NS_CORE_CRYPTO_PBKDF2_CMAC_H_

#include "core/core-config.h"
#include <stdbool.h>
#include <stdint.h>

#define NS_PBKDF2_SALT_MAX_LEN 30 // salt prefix (6) + extended panid (8) + network name (16)

void
ns_pbkdf2_cmac(const uint8_t *password,
               uint16_t password_len,
               const uint8_t *salt,
               uint16_t salt_len,
               uint32_t iteration_counter,
               uint16_t akey_len,
               uint8_t *akey);

#endif // NS_CORE_CRYPTO_PBKDF2_CMAC_H_
