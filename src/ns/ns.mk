# Network stack core makefile

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

SRC_NS_CORE_API += $(addprefix ns/sys/core/api/,\
    instance_api.c \
    nstd_api.c \
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

# compiler flags defintions for build Nespy stack
CFLAGS += -DNS_RADIO=1

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CORE:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CLI:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_TEST:.c=.o))
