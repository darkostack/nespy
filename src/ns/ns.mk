# Network stack core makefile

SRC_NS_CORE += $(addprefix ns/sys/core/,\
    common/instance.c \
    common/timer.c \
    api/instance_api.c \
    api/nstd_api.c \
    )

SRC_NS_CLI += $(addprefix ns/sys/cli/,\
    cli.c \
    cli-uart.c \
    )

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CORE:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CLI:.c=.o))
