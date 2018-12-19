# Network stack core makefile
########################################
# BUILD OPTIONS
########################################
# set 1 to use radio only mode
NS_RADIO=1

# set 1 to enable raw link api
NS_ENABLE_RAW_LINK_API=0

########################################

SRC_NS_CORE_COMMON += $(addprefix ns/sys/core/common/,\
    instance.c \
    timer.c \
    tasklet.c \
    message.c \
    encoding.c \
    random.c \
    string.c \
    trickle_timer.c \
    )

SRC_NS_CORE_UTILS += $(addprefix ns/sys/core/utils/,\
    heap.c \
    )

SRC_NS_CORE_MAC += $(addprefix ns/sys/core/mac/,\
    mac_frame.c \
    channel_mask.c \
    )

ifeq ($(NS_RADIO),1)
SRC_NS_CORE_MAC += ns/sys/core/mac/link_raw.c
endif

ifeq ($(NS_ENABLE_RAW_LINK_API),1)
SRC_NS_CORE_MAC += ns/sys/core/mac/link_raw.c
endif

SRC_NS_CORE_API += $(addprefix ns/sys/core/api/,\
    instance_api.c \
    tasklet_api.c \
    message_api.c \
    )

SRC_NS_CORE += $(SRC_NS_CORE_COMMON)
SRC_NS_CORE += $(SRC_NS_CORE_UTILS)
SRC_NS_CORE += $(SRC_NS_CORE_MAC)
SRC_NS_CORE += $(SRC_NS_CORE_API)

SRC_NS_CLI += $(addprefix ns/sys/cli/,\
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
    )

# compiler flags defintions based on BUILD OPTIONS
CFLAGS += -DNS_RADIO=$(NS_RADIO)
CFLAGS += -DNS_ENABLE_RAW_LINK_API=$(NS_ENABLE_RAW_LINK_API)

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CORE:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CLI:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_TEST:.c=.o))
