BOARD ?= nespy1

# check the board
ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified: boards/$(BOARD))
endif

# if soft-device is selected, try to use that one.
SD ?=
SD_LOWER = $(shell echo $(SD) | tr '[:upper:]' '[:lower:]')

include boards/$(BOARD)/mpconfigboard.mk

ifeq ($(SD), )
BUILD ?= build/$(BOARD)
include ../../py/mkenv.mk
else
BUILD ?= build/$(BOARD)-$(SD_LOWER)
include ../../py/mkenv.mk
LD_FILES += boards/$(SD_LOWER)_$(SOFTDEV_VERSION).ld
include drivers/bluetooth/bluetooth_common.mk
endif

LD_FILES += boards/memory.ld boards/common.ld

ifneq ($(LD_FILE),)
# use this custom ld file
LD_FILES += $(LD_FILE)
endif

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h

# include py core make definitions
include ../../py/py.mk

MICROPY_FATFS ?= 0
FATFS_DIR = lib/oofatfs

CROSS_COMPILE = arm-none-eabi-

INC += -I.
INC += -I../..
INC += -I./$(BUILD)
INC += -I../../lib/cmsis/inc
INC += -I../../lib/nrfx
INC += -I../../lib/nrfx/drivers
INC += -I../../lib/nrfx/drivers/include
INC += -I../../lib/nrfx/mdk
INC += -I../../lib/nrfx/hal
INC += -I./modules/machine
INC += -I./modules/ubluepy
INC += -I./modules/music
INC += -I./modules/random
INC += -I./modules/ble
INC += -I./modules/board
INC += -I./drivers/bluetooth
INC += -I./drivers
INC += -I./boards/$(BOARD)
INC += -I./nsport

MCU_VARIANT_UPPER = $(shell echo $(MCU_VARIANT) | tr '[:lower:]' '[:upper:]')
MCU_SUB_VARIANT_UPPER = $(shell echo $(MCU_SUB_VARIANT) | tr '[:lower:]' '[:upper:]')

SRC_LIB_NRFX_SYSTEM:=
ifeq ($(MCU_SUB_VARIANT),nrf52840)
SRC_LIB_NRFX_SYSTEM += $(addprefix lib/nrfx/mdk/, system_nrf52840.c)
endif

NRF_DEFINES += -D$(MCU_SUB_VARIANT_UPPER)
NRF_DEFINES += -DCONFIG_GPIO_AS_PINRESET

CFLAGS_CORTEX_M = -mthumb -mabi=aapcs -fsingle-precision-constant -Wdouble-promotion

CFLAGS_MCU_m4 = $(CFLAGS_CORTEX_M) -mtune=cortex-m4 -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard

LTO ?= 0
ifeq ($(LTO),1)
CFLAGS += -flto
else
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections
endif

CFLAGS += $(CFLAGS_MCU_$(MCU_SERIES))
CFLAGS += $(INC) -Wall -Werror -g -ansi -std=c11 -nostdlib $(COPT) $(NRF_DEFINES) $(CFLAGS_MOD)
CFLAGS += -fno-strict-aliasing
CFLAGS += -DNRF5_HAL_H='<$(MCU_VARIANT)_hal.h>'

LDFLAGS = $(CFLAGS)
LDFLAGS += -Xlinker -Map=$(@:.elf=.map)
LDFLAGS += -mthumb -mabi=aapcs $(addprefix -T,$(LD_FILES)) -L boards/

#Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
LDFLAGS += -O0
else
CFLAGS += -Os -DNDEBUG
LDFLAGS += -Os
endif

LIBS = \

ifeq ($(MCU_VARIANT), nrf52)
LIBGCC_FILE_NAME = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
LIBS += -L $(dir $(LIBGCC_FILE_NAME)) -lgcc

SRC_LIB += $(addprefix lib/,\
    libm/math.c \
    libm/fmodf.c \
    libm/nearbyintf.c \
    libm/ef_sqrt.c \
    libm/kf_rem_pio2.c \
    libm/kf_sin.c \
    libm/kf_cos.c \
    libm/kf_tan.c \
    libm/ef_rem_pio2.c \
    libm/sf_sin.c \
    libm/sf_cos.c \
    libm/sf_tan.c \
    libm/sf_frexp.c \
    libm/sf_modf.c \
    libm/sf_ldexp.c \
    libm/asinfacosf.c \
    libm/atanf.c \
    libm/atan2f.c \
    )
endif

SRC_LIB += $(addprefix lib/,\
    libc/string0.c \
    mp-readline/readline.c \
    utils/pyexec.c \
    utils/interrupt_char.c \
    timeutils/timeutils.c \
    )

ifeq ($(MICROPY_FATFS), 1)
SRC_LIB += $(addprefix lib/,\
    oofatfs/ff.c \
    oofatfs/option/unicode.c \
    )
endif

SRC_LIB_NRFX += $(addprefix lib/nrfx/drivers/src/,\
    prs/nrfx_prs.c \
    nrfx_uart.c \
    nrfx_adc.c \
    nrfx_saadc.c \
    nrfx_rng.c \
    nrfx_twi.c \
    nrfx_spi.c \
    nrfx_spim.c \
    nrfx_rtc.c \
    nrfx_timer.c \
    nrfx_pwm.c \
    nrfx_gpiote.c \
    nrfx_systick.c \
    )

SRC_LIB_NRFX_HAL += $(addprefix lib/nrfx/hal/,\
    nrf_nvmc.c \
    )

SRC_NRF += \
    main.c \
    mphalport.c \
    help.c \
    gccollect.c \
    pin_named_pins.c \
    fatfs_port.c \

SRC_NRF_DRIVERS += $(addprefix drivers/,\
    flash.c \
    softpwm.c \
    ticker.c \
    bluetooth/ble_drv.c \
    bluetooth/ble_uart.c \
    )

SRC_NRF_MODULES += $(addprefix modules/,\
    machine/modmachine.c \
    machine/uart.c \
    machine/spi.c \
    machine/i2c.c \
    machine/adc.c \
    machine/pin.c \
    machine/timer.c \
    machine/rtcounter.c \
    machine/pwm.c \
    machine/temp.c \
    uos/moduos.c \
    uos/microbitfs.c \
    utime/modutime.c \
    board/modboard.c \
    board/led.c \
    ubluepy/modubluepy.c \
    ubluepy/ubluepy_peripheral.c \
    ubluepy/ubluepy_service.c \
    ubluepy/ubluepy_uuid.c \
    ubluepy/ubluepy_delegate.c \
    ubluepy/ubluepy_constants.c \
    ubluepy/ubluepy_descriptor.c \
    ubluepy/ubluepy_scanner.c \
    ubluepy/ubluepy_scan_entry.c \
    music/modmusic.c \
    music/musictunes.c \
    ble/modble.c \
    random/modrandom.c \
    )

SRC_NRF_STARTUP += $(addprefix device/,\
    startup_$(MCU_SUB_VARIANT).c \
    )

ifneq ($(FROZEN_MPY_DIR),)
FROZEN_MPY_PY_FILES := $(shell find -L $(FROZEN_MPY_DIR) -type f -name '*.py')
FROZEN_MPY_MPY_FILES := $(addprefix $(BUILD)/,$(FROZEN_MPY_PY_FILES:.py=.mpy))
endif

# nrf porting to network stack sourcefiles
SRC_NRF_NSPORT += $(addprefix nsport/,\
    random.c \
    slip-arch.c \
    clock.c \
    rtimer-arch.c \
    int-master.c \
    watchdog.c \
    )

# network stack micropython objects
SRC_NS_MODULES += $(addprefix ns/modules/,\
    modns.c \
    process.c \
    )

# include nrf project-conf for network stack
CFLAGS += -DPROJECT_CONF_PATH=\"project-conf.h\"

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/,$(SRC_LIB:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_LIB_NRFX:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_LIB_NRFX_HAL:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_LIB_NRFX_SYSTEM:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NRF:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NRF_DRIVERS:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NRF_MODULES:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NRF_STARTUP:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NRF_NSPORT:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_MODULES:.c=.o))
OBJ += $(BUILD)/pins_gen.o

# include ns networking core makefile
include ../../ns/ns.mk

$(BUILD)/$(FATFS_DIR)/ff.o: COPT += -Os
$(filter $(PY_BUILD)/../extmod/vfs_fat_%.o, $(PY_O)): COPT += -Os

.PHONY: all binary hex

all: binary hex

OUTPUT_FILENAME = firmware-$(BOARD)

# create binary .bin file from the .out file
binary: $(BUILD)/$(OUTPUT_FILENAME).bin

$(BUILD)/$(OUTPUT_FILENAME).bin: $(BUILD)/$(OUTPUT_FILENAME).elf
	$(OBJCOPY) -O binary $< $@

# create binary .hex file from the .out file
hex: $(BUILD)/$(OUTPUT_FILENAME).hex

$(BUILD)/$(OUTPUT_FILENAME).hex: $(BUILD)/$(OUTPUT_FILENAME).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/$(OUTPUT_FILENAME).elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)
	$(Q)$(SIZE) $@

# List of sources for qstr extraction
SRC_QSTR += $(SRC_NRF) $(SRC_NRF_DRIVERS) $(SRC_NRF_MODULES) $(SRC_NS_MODULES)

# Append any auto-generated sources that are needed by sources listed in SRC_QSTR
SRC_QSTR_AUTO_DEPS +=

# Making OBJ use an order-only depenedency on the generated pins.h file
# has the side effect of making the pins.h file before we actually compile
# any of the objects. The normal dependency generation will deal with the
# case when pins.h is modified. But when it doesn't exist, we don't know
# which source files might need it.
$(OBJ): | $(HEADER_BUILD)/pins.h

# Use a pattern rule here so that make will only call make-pins.py once to make
# both pins_gen.c and pins.h
$(BUILD)/%_gen.c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(BUILD)/%_qstr.h: boards/$(BOARD)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) --prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) --qstr $(GEN_PINS_QSTR) --af-const $(GEN_PINS_AF_CONST) --af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)

$(BUILD)/pins_gen.o: $(BUILD)/pins_gen.c
	$(call compile_c)

MAKE_PINS = boards/make-pins.py
BOARD_PINS = boards/$(BOARD)/pins.csv
AF_FILE = $(MCU_VARIANT)_af.csv
PREFIX_FILE = boards/$(MCU_VARIANT)_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_gen.c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

ifneq ($(FROZEN_DIR),)
# To use frozen source modules, put your .py files in a subdirectory (eg scripts/)
# and then invoke make with FROZEN_DIR=scripts (be sure to build from scratch).
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif

ifneq ($(FROZEN_MPY_DIR),)
# To use frozen bytecode, put your .py files in a subdirectory (eg frozen/) and
# then invoke make with FROZEN_MPY_DIR=frozen (be sure to build from scratch).
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
endif

$(PY_BUILD)/nlr%.o: CFLAGS += -Os -fno-lto

include ../../py/mkrules.mk

