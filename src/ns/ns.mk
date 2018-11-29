# Network stack core makefile

SRC_NS_CORE += $(addprefix ns/sys/core/,\
    common/instance.c \
    common/timer.c \
    common/tasklet.c \
    common/message.c \
    mac/mac_frame.c \
    utils/heap.c \
    api/instance_api.c \
    api/nstd_api.c \
    api/tasklet_api.c \
    api/message_api.c \
    )

SRC_NS_CLI += $(addprefix ns/sys/cli/,\
    cli.c \
    cli-uart.c \
    )

SRC_NS_TEST += $(addprefix ns/test/unit/,\
    message.c \
    message_queue.c \
    priority_queue.c \
    heap.c \
    )

# compiler flags defintions for build Nespy stack
CFLAGS += -DNS_RADIO=1

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CORE:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CLI:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_TEST:.c=.o))
