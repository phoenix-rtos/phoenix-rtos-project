CC=arm-elf-gcc
LD=arm-elf-gcc
AIS=-marm
TIS=-mthumb
CFLAGS=-mcpu=arm7tdmi -mthumb-interwork -ffreestanding -O2 -c
LFLAGS=-mthumb-interwork -ffreestanding -T./lpc2148_flash.ld -nostartfiles

SRCS = plo.c low.c plostd.c cmd.c timer.c serial.c iap.c msg.c phfs.c phoenixd.c exceptions.c
OBJS =  _startup.o _low.o $(SRCS:.c=.o)


all: arm-plo.hex

.c.o:
	$(CC) $(TIS) $(CFLAGS) $<

.S.o:
	$(CC) $(AIS) $(CFLAGS) $<

exceptions.o: exceptions.c
	$(CC) $(AIS) $(CFLAGS) exceptions.c

arm-plo.hex: arm-plo.elf
	arm-elf-objcopy -O ihex arm-plo.elf arm-plo.hex

arm-plo.elf: $(OBJS)
	$(LD) $(LFLAGS) -o arm-plo.elf $(OBJS)

clean:
	rm *.o *.elf *.hex
