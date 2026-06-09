# Inclua este arquivo nos Makefiles de cada entrega com:
#   include ../utils/Makefile.inc

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

UTILS   := $(dir $(lastword $(MAKEFILE_LIST)))

CPU     = -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft
CFLAGS  = $(CPU) -Os -Wall -Wextra -ffunction-sections -fdata-sections -nostartfiles
LDFLAGS = $(CPU) -T $(UTILS)kl25z.ld -Wl,--gc-sections -Wl,-Map=output.map -nostartfiles

SRCS   += $(UTILS)startup.c
TARGET  = output

all: $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(TARGET).elf $(TARGET).bin output.map

.PHONY: all clean
