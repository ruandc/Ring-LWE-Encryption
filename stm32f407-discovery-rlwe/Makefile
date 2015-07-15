TARGET=main.hex
EXECUTABLE=main.elf
BINTARGET=main.bin
TOOLCHAIN_PATH=~/localdisk/toolchains/gcc-arm-none-eabi-4_8-2014q2/bin
STFLASH=/users/cosic/rdeclerc/localdisk/toolchains/stlink/install/bin/st-flash
STUTIL=/users/cosic/rdeclerc/localdisk/toolchains/stlink/install/bin/st-util


CC=${TOOLCHAIN_PATH}/arm-none-eabi-gcc
LD=${TOOLCHAIN_PATH}/arm-none-eabi-gcc
AR=${TOOLCHAIN_PATH}/arm-none-eabi-ar
AS=${TOOLCHAIN_PATH}/arm-none-eabi-as
CP=${TOOLCHAIN_PATH}/arm-none-eabi-objcopy
OD=${TOOLCHAIN_PATH}/arm-none-eabi-objdump

BIN=$(CP) -O ihex

STD_PERIPH=/users/cosic/rdeclerc/templates/STM32F4-workarea/Libraries/STM32F4xx_StdPeriph_Driver
#STD_PERIPH=/home/rdeclerc/templates/STM32F4-workarea/Libraries/STM32F4xx_StdPeriph_Driver-g
CMSIS=/users/cosic/rdeclerc/templates/STM32F4-workarea/Libraries/CMSIS
#CMSIS=/home/rdeclerc/templates/STM32F4-workarea/Libraries/CMSIS

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DHSE_VALUE=8000000
STARTUP = ./startup_stm32f40xx.s

MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork -std=c99 -g3
STM32_INCLUDES = -I$(CMSIS)/Device/ST/STM32F4xx/Include/ \
	-I$(CMSIS)/Include/ \
	-I$(STD_PERIPH)/inc

OPTIMIZE       = -O0

CFLAGS	= $(MCFLAGS)  $(OPTIMIZE)  $(DEFS) -I. -I./ $(STM32_INCLUDES)  -Wl,-T,stm32_flash.ld
AFLAGS	= $(MCFLAGS) 
#-mapcs-float use float regs. small increase in code size

SRC = main.c \
	stm32f4xx_it.c \
	system_stm32f4xx.c \
	lwe.c \
	knuth_yao_asm.S \
	knuth_yao_asm_shuffle.S \
	test_asm.S \
	test_asm.c \
	term_io.c \
	comm.c \
	speed_test.c \
	global.c \
	unit_test.c \
	$(STD_PERIPH)/src/misc.c \
	$(STD_PERIPH)/src/stm32f4xx_adc.c \
	$(STD_PERIPH)/src/stm32f4xx_can.c \
	$(STD_PERIPH)/src/stm32f4xx_crc.c \
	$(STD_PERIPH)/src/stm32f4xx_cryp_aes.c \
	$(STD_PERIPH)/src/stm32f4xx_cryp.c \
	$(STD_PERIPH)/src/stm32f4xx_cryp_des.c \
	$(STD_PERIPH)/src/stm32f4xx_cryp_tdes.c \
	$(STD_PERIPH)/src/stm32f4xx_dac.c \
	$(STD_PERIPH)/src/stm32f4xx_dbgmcu.c \
	$(STD_PERIPH)/src/stm32f4xx_dcmi.c \
	$(STD_PERIPH)/src/stm32f4xx_dma.c \
	$(STD_PERIPH)/src/stm32f4xx_exti.c \
	$(STD_PERIPH)/src/stm32f4xx_flash.c \
	$(STD_PERIPH)/src/stm32f4xx_fsmc.c \
	$(STD_PERIPH)/src/stm32f4xx_gpio.c \
	$(STD_PERIPH)/src/stm32f4xx_hash.c \
	$(STD_PERIPH)/src/stm32f4xx_hash_md5.c \
	$(STD_PERIPH)/src/stm32f4xx_hash_sha1.c \
	$(STD_PERIPH)/src/stm32f4xx_i2c.c \
	$(STD_PERIPH)/src/stm32f4xx_iwdg.c \
	$(STD_PERIPH)/src/stm32f4xx_pwr.c \
	$(STD_PERIPH)/src/stm32f4xx_rcc.c \
	$(STD_PERIPH)/src/stm32f4xx_rng.c \
	$(STD_PERIPH)/src/stm32f4xx_rtc.c \
	$(STD_PERIPH)/src/stm32f4xx_sdio.c \
	$(STD_PERIPH)/src/stm32f4xx_spi.c \
	$(STD_PERIPH)/src/stm32f4xx_syscfg.c \
	$(STD_PERIPH)/src/stm32f4xx_tim.c \
	$(STD_PERIPH)/src/stm32f4xx_usart.c \
	$(STD_PERIPH)/src/stm32f4xx_wwdg.c 

OBJDIR = .
OBJ = $(SRC:%.c=$(OBJDIR)/%.o)
OBJ = $(OBJ:%.s=$(OBJDIR)/%.o)
OBJ = $(OBJ:%.S=$(OBJDIR)/%.o)    
OBJ += Startup.o

all: $(TARGET)

$(BINTARGET):all
	$(CP) -O binary $(EXECUTABLE) $(BINTARGET)

$(TARGET): $(EXECUTABLE)
	$(CP) -O ihex $^ $@

$(EXECUTABLE): $(SRC) $(STARTUP)
	$(CC) $(CFLAGS) $^ -lm -lc -lnosys -o $@

clean:
# rm -f Startup.lst  $(TARGET)  $(TARGET).lst $(OBJ) $(AUTOGEN)  $(TARGET).out  $(TARGET).hex  $(TARGET).map  $(TARGET).dmp  $(TARGET).elf
	rm -f main.elf main.hex main.elf.dump main.bin

program: $(BINTARGET)
	${STFLASH} --reset write $(BINTARGET) 0x8000000
	
disassemble: $(TARGET)
	arm-none-eabi-objdump $(EXECUTABLE) --disassemble > $(EXECUTABLE).dump
	
uart:
	make clean
	make program
	screen /dev/ttyUSB0 9600

acm0:
	screen /dev/ttyACM0 9600
	
gdb-server:
	${STUTIL}
