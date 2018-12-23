# Network stack core makefile
########################################
# BUILD OPTIONS
########################################
NS_RADIO=0
NS_ENABLE_RAW_LINK_API=0
NS_ENABLE_MAC_FILTER=0

########################################

SRC_NS_CORE_COMMON += $(addprefix ns/src/core/common/,\
    instance.c \
    timer.c \
    tasklet.c \
    message.c \
    encoding.c \
    random.c \
    string.c \
    trickle_timer.c \
    logging.c \
    notifier.c \
    )

SRC_NS_CORE_UTILS += $(addprefix ns/src/core/utils/,\
    heap.c \
    )

SRC_NS_CORE_MAC += $(addprefix ns/src/core/mac/,\
    mac_frame.c \
    channel_mask.c \
    link_raw.c \
    mac_filter.c \
    mac.c \
    )

SRC_NS_CORE_THREAD += $(addprefix ns/src/core/thread/,\
    link_quality.c \
    )

SRC_NS_CORE_CRYPTO += $(addprefix ns/src/core/crypto/,\
    aes_ccm.c \
    aes_ecb.c \
    mbedtls.c \
    )

SRC_NS_CORE_API += $(addprefix ns/src/core/api/,\
    instance_api.c \
    tasklet_api.c \
    message_api.c \
    logging_api.c \
    )

SRC_NS_CORE += $(SRC_NS_CORE_COMMON)
SRC_NS_CORE += $(SRC_NS_CORE_UTILS)
SRC_NS_CORE += $(SRC_NS_CORE_MAC)
SRC_NS_CORE += $(SRC_NS_CORE_THREAD)
SRC_NS_CORE += $(SRC_NS_CORE_CRYPTO)
SRC_NS_CORE += $(SRC_NS_CORE_API)

SRC_NS_CLI += $(addprefix ns/src/cli/,\
    cli.c \
    cli-uart.c \
    )

SRC_NS_TEST += $(addprefix ns/test/unit/,\
    test_message.c \
    test_message_queue.c \
    test_priority_queue.c \
    test_heap.c \
    test_string.c \
    test_mac_frame.c \
    test_link_quality.c \
    test_logging.c \
    test_aes.c \
    )

# include path
CFLAGS += -I$(TOP)/ns/include
CFLAGS += -I$(TOP)/ns/src

# test code directories
CFLAGS += -I$(TOP)/ns/test/unit

# compiler flags defintions based on BUILD OPTIONS
CFLAGS += -DNS_RADIO=$(NS_RADIO)
CFLAGS += -DNS_ENABLE_RAW_LINK_API=$(NS_ENABLE_RAW_LINK_API)
CFLAGS += -DNS_ENABLE_MAC_FILTER=$(NS_ENABLE_MAC_FILTER)

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CORE:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CLI:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_TEST:.c=.o))

# include third party makefile
include $(TOP)/ns/third_party/mbedtls/mbedtls.mk