SRC_NS_THIRD_PARTY_EMBEDTLS += $(addprefix ns/third_party/mbedtls/,\
    repo/library/aes.c \
    repo/library/asn1parse.c \
    repo/library/asn1write.c \
    repo/library/base64.c \
    repo/library/bignum.c \
    repo/library/ccm.c \
    repo/library/cipher.c \
    repo/library/cipher_wrap.c \
    repo/library/cmac.c \
    repo/library/ctr_drbg.c \
    repo/library/debug.c \
    repo/library/ecdh.c \
    repo/library/ecdsa.c \
    repo/library/ecjpake.c \
    repo/library/ecp_curves.c \
    repo/library/entropy.c \
    repo/library/entropy_poll.c \
    repo/library/md.c \
    repo/library/md_wrap.c \
    repo/library/memory_buffer_alloc.c \
    repo/library/oid.c \
    repo/library/pem.c \
    repo/library/pk.c \
    repo/library/pk_wrap.c \
    repo/library/pkparse.c \
    repo/library/platform.c \
    repo/library/platform_util.c \
    repo/library/sha256.c \
    repo/library/ssl_cookie.c \
    repo/library/ssl_ciphersuites.c \
    repo/library/ssl_cli.c \
    repo/library/ssl_srv.c \
    repo/library/ssl_ticket.c \
    repo/library/ssl_tls.c \
    repo/library/x509.c \
    repo/library/x509_crt.c \
    )

CFLAGS += -I$(TOP)/ns/third_party/mbedtls
CFLAGS += -I$(TOP)/ns/third_party/mbedtls/repo/include
CFLAGS += -DMBEDTLS_CONFIG_FILE=\"mbedtls-config.h\"

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_THIRD_PARTY_EMBEDTLS:.c=.o))
