#include "core/crypto/ecdsa.h"
#include "core/common/code_utils.h"
#include "core/common/debug.h"
#include "core/common/random.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/pk.h"

#if NS_ENABLE_ECDSA
static int
ecdsa_fill_random(void *ptr, unsigned char *buffer, size_t size)
{
    (void)ptr;
    random_fill_buffer(buffer, size);
    return 0;
}

static ns_error_t
ecdsa_sign(uint8_t *output,
           uint16_t *output_length,
           const uint8_t *input_hash,
           uint16_t input_hash_length,
           const uint8_t *private_key,
           uint16_t private_key_length)
{
    ns_error_t error = NS_ERROR_NONE;
    mbedtls_ecdsa_context ctx;
    mbedtls_pk_context pk_ctx;
    mbedtls_ecp_keypair *keypair;
    mbedtls_mpi rmpi;
    mbedtls_mpi smpi;

    mbedtls_pk_init(&pk_ctx);
    mbedtls_ecdsa_init(&ctx);

    // parse a private key in PEM format
    VERIFY_OR_EXIT(mbedtls_pk_parse_key(&pk_ctx, private_key, private_key_length, NULL, 0) == 0,
                   error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(mbedtls_pk_get_type(&pk_ctx) == MBEDTLS_PK_ECKEY, error = NS_ERROR_INVALID_ARGS);

    keypair = mbedtls_pk_ec(pk_ctx);
    assert(keypair != NULL);

    VERIFY_OR_EXIT(mbedtls_ecdsa_from_keypair(&ctx, keypair) == 0, error = NS_ERROR_FAILED);

    mbedtls_mpi_init(&rmpi);
    mbedtls_mpi_init(&smpi);

    // sign using ECDSA
    VERIFY_OR_EXIT(mbedtls_ecdsa_sign(&ctx.grp, &rmpi, &smpi, &ctx.d, input_hash, input_hash_length, ecdsa_fill_random, NULL) == 0, error = NS_ERROR_FAILED);
    VERIFY_OR_EXIT(mbedtls_mpi_size(&rmpi) + mbedtls_mpi_size(&smpi) <= *output_length, error = NS_ERROR_NO_BUFS);

    // concatenate the two octet sequences in the order R and then S
    VERIFY_OR_EXIT(mbedtls_mpi_write_binary(&rmpi, output, mbedtls_mpi_size(&rmpi)) == 0, error = NS_ERROR_FAILED);
    *output_length = mbedtls_mpi_size(&rmpi);

    VERIFY_OR_EXIT(mbedtls_mpi_write_binary(&smpi, output + *output_length, mbedtls_mpi_size(&smpi)) == 0,
                   error = NS_ERROR_FAILED);
    *output_length += mbedtls_mpi_size(&smpi);

exit:
    mbedtls_mpi_free(&rmpi);
    mbedtls_mpi_free(&smpi);
    mbedtls_ecdsa_free(&ctx);
    mbedtls_pk_free(&pk_ctx);
    return error;
}

void
crypto_ecdsa_ctor(crypto_ecdsa_t *ecdsa)
{
    ecdsa->sign = &ecdsa_sign;
}
#endif // NS_ENABLE_ECDSA
