TOOLCHAIN_PATH ?= /opt/wch/RISC-V\ Embedded\ GCC/bin
OPENOCD_PATH   ?= /opt/wch/OpenOCD/bin
PROJECT_NAME    =  ch32v003_bare_metal_spi_ili9341

TOP_DIR     := .
OUTPUT_DIR  := $(TOP_DIR)/build
CORE_DIR    := $(TOP_DIR)/core
USER_DIR    := $(TOP_DIR)/src
DRIVERS_DIR	:= $(TOP_DIR)/drivers

LD_FILE     := $(TOP_DIR)/ch32v003_FLASH.ld
MAP_FILE    := $(OUTPUT_DIR)/$(PROJECT_NAME).map
ELF_FILE    := $(OUTPUT_DIR)/$(PROJECT_NAME).elf
HEX_FILE    := $(OUTPUT_DIR)/$(PROJECT_NAME).hex
LST_FILE    := $(OUTPUT_DIR)/$(PROJECT_NAME).lst
SIZ_FILE    := $(OUTPUT_DIR)/$(PROJECT_NAME).siz

INCLUDES := $(INCLUDES)
INCLUDES += -I $(TOP_DIR)/inc
INCLUDES += -I $(TOP_DIR)/core
INCLUDES += -I $(DRIVERS_DIR)

CCFLAGS := -march=rv32ec \
           -mabi=ilp32e \
           -msmall-data-limit=0 \
           -msave-restore \
           -Os \
           -fmessage-length=0 \
           -fsigned-char \
           -ffunction-sections \
           -fdata-sections \
		   -fno-common \
           -Wunused -Wuninitialized -g

all: $(HEX_FILE) $(LST_FILE) $(SIZ_FILE)

STARTUP_SRCS := $(wildcard $(TOP_DIR)/*.S)
STARTUP_OBJS := $(patsubst $(TOP_DIR)/%.S, $(OUTPUT_DIR)/%.o, $(STARTUP_SRCS))

CORE_SRC := $(wildcard $(CORE_DIR)/*.c)
CORE_OBJS := $(patsubst $(CORE_DIR)/%.c, $(OUTPUT_DIR)/core/%.o, $(CORE_SRC))

USER_SRCS := $(wildcard $(USER_DIR)/*.c)
USER_OBJS := $(patsubst $(USER_DIR)/%.c, $(OUTPUT_DIR)/src/%.o, $(USER_SRCS))

DRIVERS_SRCS := $(wildcard $(DRIVERS_DIR)/*.c)
DRIVERS_OBJS := $(patsubst $(DRIVERS_DIR)/%.c, $(OUTPUT_DIR)/drivers/%.o, $(DRIVERS_SRCS))


$(OUTPUT_DIR)/%.o: $(TOP_DIR)/%.S
	@mkdir -p $(@D)
	$(TOOLCHAIN_PATH)/riscv-none-embed-gcc $(CCFLAGS) -x assembler -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/core/%.o: $(CORE_DIR)/%.c
	@mkdir -p $(@D)
	$(TOOLCHAIN_PATH)/riscv-none-embed-gcc $(CCFLAGS) $(INCLUDES) -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"


$(OUTPUT_DIR)/src/%.o: $(USER_DIR)/%.c
	@mkdir -p $(@D)
	$(TOOLCHAIN_PATH)/riscv-none-embed-gcc $(CCFLAGS) $(INCLUDES) -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/drivers/%.o: $(DRIVERS_DIR)/%.c
	@mkdir -p $(@D)
	$(TOOLCHAIN_PATH)/riscv-none-embed-gcc $(CCFLAGS) $(INCLUDES) -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(ELF_FILE): $(STARTUP_OBJS) $(CORE_OBJS) $(USER_OBJS) $(DRIVERS_OBJS)
	$(TOOLCHAIN_PATH)/riscv-none-embed-gcc $(CCFLAGS) -T $(LD_FILE) -nostartfiles -Xlinker --gc-sections -Wl,-Map,$(MAP_FILE) --specs=nano.specs --specs=nosys.specs -o $(ELF_FILE) $(USER_OBJS) $(STARTUP_OBJS) $(CORE_OBJS) $(DRIVERS_OBJS)

$(HEX_FILE): $(ELF_FILE)
	$(TOOLCHAIN_PATH)/riscv-none-embed-objcopy -O ihex $(ELF_FILE) $(HEX_FILE)

$(LST_FILE): $(ELF_FILE)
	$(TOOLCHAIN_PATH)/riscv-none-embed-objdump --all-headers --demangle --disassemble $(ELF_FILE) > $(LST_FILE)

$(SIZ_FILE): $(ELF_FILE)
	$(TOOLCHAIN_PATH)/riscv-none-embed-size --format=berkeley $(ELF_FILE)


.PHONY: clean
clean:
	rm -f $(OUTPUT_DIR)/core/*
	rm -f $(OUTPUT_DIR)/src/*
	rm -f $(OUTPUT_DIR)/drivers/*
	rm -f $(OUTPUT_DIR)/*.*

erase:
	$(OPENOCD_PATH)/openocd -f $(OPENOCD_PATH)/wch-riscv.cfg -c init -c halt -c "flash erase_sector wch_riscv 0 last" -c exit

flash:
	$(OPENOCD_PATH)/openocd -f $(OPENOCD_PATH)/wch-riscv.cfg $(OPENOCD_CFG) -c init -c halt -c "flash erase_sector wch_riscv 0 last " -c "program $(ELF_FILE)" -c "verify_image $(ELF_FILE)" -c reset -c resume -c exit

reset:
	$(OPENOCD_PATH)/openocd -f $(OPENOCD_PATH)/wch-riscv.cfg $(OPENOCD_CFG) -c init -c reset -c resume -c exit

