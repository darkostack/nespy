# Network stack core makefile

SRC_NS_CORE += $(addprefix ns/sys/core/,\
    common/instance.c \
    common/timer.c \
    api/instance_api.c \
    api/instance_api.c \
    )

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_CORE:.c=.o))
