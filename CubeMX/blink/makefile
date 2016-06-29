#Set your library location here

STM=/usr/local/stm32/library

#Set the path of your gnarm here
CCPATH = /usr/local/gnuarm/gcc-arm-none-eabi/bin

#Set output dir here:
OUTDIR = .

#Set target here:
TARGET= blink

#Set the startup file according the type of your board
STARTUP = $(STM)/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xB.s

#define your type of board
DEFS = -DSTM32F103xB 

#set your link file
LINKER= $(STM)/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/linker/STM32F103XB_FLASH.ld

CC=$(CCPATH)/arm-none-eabi-gcc
OBJDUMP=$(CCPATH)/arm-none-eabi-objdump
OBJCOPY=$(CCPATH)/arm-none-eabi-objcopy

FLAGS = -mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork

INCLUDES = -I $(STM)/CMSIS/Device/ST/STM32F1xx/Include \
	-I $(STM)/CMSIS/Include \
	-I $(STM)/STM32F1xx_HAL_Driver/Inc \
	-I $(STM)/BSP/STM32F1xx_Nucleo/ \
	-I ./Inc/ 

CON_INC = $(STM)/STM32F1xx_HAL_Driver/Src


SRC = ./Src/main.c \
	./Src/stm32f1xx_it.c \
	$(STM)/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c \
	$(CON_INC)/stm32f1xx_hal_gpio.c \
	$(CON_INC)/stm32f1xx_hal_rcc.c \
	$(CON_INC)/stm32f1xx_hal.c \
	$(CON_INC)/stm32f1xx_hal_cortex.c


all: $(OUTDIR)/$(TARGET).hex $(OUTDIR)/$(TARGET).list $(OUTDIR)/$(TARGET).dump

$(OUTDIR)/$(TARGET).elf: $(SRC) $(STARTUP)
	$(CC) $(FLAGS) $(DEFS) -I. $(INCLUDES)   -Wl,-T,$(LINKER)  $^   -lnosys -o $@
$(OUTDIR)/$(TARGET).hex: $(OUTDIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $(OUTDIR)/$(TARGET).elf  $(OUTDIR)/$(TARGET).hex

$(OUTDIR)/$(TARGET).list: $(OUTDIR)/$(TARGET).elf
	$(OBJDUMP) -D $(OUTDIR)/$(TARGET).elf > $(OUTDIR)/$(TARGET).list

$(OUTDIR)/$(TARGET).dump: $(OUTDIR)/$(TARGET).elf
	$(OBJDUMP) -s $(OUTDIR)/$(TARGET).elf > $(OUTDIR)/$(TARGET).dump

clean: 
	rm -rf $(OUTDIR)/*.hex $(OUTDIR)/*.elf $(OUTDIR)/*.dump $(OUTDIR)/*.o $(OUTDIR)/*.list

