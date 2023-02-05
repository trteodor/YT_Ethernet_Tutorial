
TARGET = eth_tut
DEBUG ?= 1
OPT = -Og
# Build path
BUILD_DIR = .build

#######################################
# LWIP INCLUDES DIRS
lwip_INC :=
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/apps
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/include/lwip/apps
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/include/lwip
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/system/arch
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/include/netif
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/include/lwip/prot
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/include/lwip/priv
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/apps/http
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/include
lwip_INC += -IMiddlewares/lwip-STABLE-2_1_3/src/system

######################################
# source
######################################
LWIPDIR := Middlewares/lwip-STABLE-2_1_3/src

include Middlewares/lwip-STABLE-2_1_3/src/Filelists.mk


# C sources
C_SOURCES += Application/main.c
C_SOURCES += Config/LWIP/App/lwip.c
C_SOURCES += Config/LWIP/Target/ethernetif.c
C_SOURCES += Application/leds.c
C_SOURCES += Application/syscalls.c
C_SOURCES += Application/sysmem.c
C_SOURCES += Drivers/GPIO/GPIO_f7.c
C_SOURCES += Drivers/System/System.c
C_SOURCES += Middlewares/DLTuc_libFiles/DLTuc.c
C_SOURCES += Drivers/usart3/UART3_dlt.c
C_SOURCES += Drivers/Ethernet_nucleof767/eth_nuc_f767.c
C_SOURCES += $(LWIPAPPFILES)
C_SOURCES += $(LWIPNOAPPSFILES)

# ASM sources
ASM_SOURCES += _startup_stm32f767zitx.s



PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7
# fpu
FPU = -mfpu=fpv5-d16
# float-abi
FLOAT-ABI = -mfloat-abi=hard
# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
# macros for gcc
# AS defines
AS_DEFS = 
# C defines
C_DEFS += -DSTM32F767xx
# AS includes
AS_INCLUDES = 
# C includes
C_INCLUDES += $(lwip_INC)
C_INCLUDES += -IDApplication
C_INCLUDES += -IDrivers/CMSIS/Device/ST/STM32F7xx/Include
C_INCLUDES += -IDrivers/CMSIS/Include
C_INCLUDES += -IDrivers/GPIO
C_INCLUDES += -IDrivers/System
C_INCLUDES += -IDrivers/usart3
C_INCLUDES += -IMiddlewares/DLTuc_libFiles
C_INCLUDES += -IDrivers/Ethernet_nucleof767
C_INCLUDES += -IConfig/LWIP
C_INCLUDES += -IConfig/LWIP/App
C_INCLUDES += -IConfig/LWIP/arch
C_INCLUDES += -IConfig/LWIP/Target
C_INCLUDES += $(Config_IncDirs)

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = _STM32F767ZITX_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


$(info -----------------------------------------------------)
$(info If You Changed HTML you should update  file "fsdata.c"!)
$(info To do it you can use command: make html)
$(info -----------------------------------------------------)

all: flash $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	@echo -----------------------------------------------------
	@echo If You Changed HTML you should update  file "fsdata.c"!
	@echo To do it you can use call the target: make html_update
	@echo -----------------------------------------------------

html_update:
	@echo HHTP Refreshing start
	@echo ------------------------
	./Tools/makedata/makefsdata.exe htmlgen AppHTML_Page -f:Config/LWIP/Http_fsdata/fsdata.c
	@echo HTTP Refreshing end
	@echo ------------------------
	@echo ------------------------

# flash: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
flash: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	openocd \
	-f interface/stlink.cfg -f target/stm32f7x.cfg \
	-c "init" -c "reset halt" \
	-c "flash write_image erase $(BUILD_DIR)/eth_tut.bin 0x8000000" \
	-c "reset" -c "shutdown"
# default action: build all




.PHONY: $(refresh_html)


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
	@echo CC $<

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo linking...
	@echo ----------------------
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		


clean:
	-rm -fR $(BUILD_DIR)
  

#\
#-c "verify_image $(BUILD_DIR)/eth_tut.bin" -c "reset" -c "shutdown"

-include $(wildcard $(BUILD_DIR)/*.d)
