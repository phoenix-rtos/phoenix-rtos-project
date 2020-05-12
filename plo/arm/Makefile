CROSS ?= arm-phoenix-

CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CFLAGS=-g -O1 -nostartfiles -nostdlib -mcpu=cortex-a5 -mtune=cortex-a5 -fomit-frame-pointer -ffreestanding -mthumb-interwork -mno-unaligned-access -c -DTARGET="$(TARGET)"
LFLAGS=-g -nostartfiles -nostdlib -T./vybrid6.ld
LIBS = -lgcc

CFLAGS += -Werror

SRCS = plo.c low.c plostd.c cmd.c timer.c serial.c iap.c msg.c phfs.c phoenixd.c exceptions.c flash.c bootrom_data.c kernel_args.c bootlog.c
SRCS += mmcblk/io/mmcblk_sdhc.c mmcblk/mmcblk_sd.c mmcblk/mmcblk.c
SRCS += sign/signature.c sign/sha1.c
OBJS =  _startup.o  $(SRCS:.c=.o)
includes = $(wildcard *.h mmcblk/*.h)

all: arm-plo.hex arm-plo.img
	

.S.o:
	$(CC) $(CFLAGS) $<

exceptions.o: exceptions.c
	$(CC) $(CFLAGS) exceptions.c

arm-plo.hex: arm-plo.elf
	$(OBJCOPY) -O ihex arm-plo.elf arm-plo.hex

arm-plo.img: arm-plo.elf
	$(OBJCOPY) -O binary arm-plo.elf arm-plo.img

arm-plo.elf: $(OBJS)
	$(CC) $(LFLAGS) -o arm-plo.elf $(OBJS) $(LIBS)

clean:
	rm -f *.o *.elf *.hex mmcblk/*.o mmcblk/io/*.o sign/*.o
