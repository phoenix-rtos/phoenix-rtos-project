SRCS := efi.c elf.c

OBJS := $(addsuffix .o,$(basename $(SRCS)))

CC := $(CROSS)gcc
LD := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

CFLAGS = -m32 -march=i586 -g -Os -Wall -DHAL=\"hal.h\" \
	-fpie -ffreestanding -fomit-frame-pointer -fno-asynchronous-unwind-tables -fshort-wchar

LDFLAGS = -T plo.lds -static

OBJFLAGS = -O pei-i386 --subsystem efi-app --stack 0x1000 --heap 0x1000 --image-base 0 --strip-all


.PHONY: all
all: plo.efi

comma = ,


%.o: %.S Makefile
	$(CC) -MMD -MF .$@.d -o $@ $(CFLAGS) -c $<

%.o: %.c Makefile
	$(CC) -MMD -MF .$@.d -o $@ $(CFLAGS) -c $<

-include $(patsubst %,.%.d,$(OBJS))


plo.coff: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ --as-needed -\( $^ -\)

plo.coff: plo.lds

plo.efi: plo.coff
	$(OBJCOPY) $(OBJFLAGS) $< $@
	echo '00000096 0e'|xxd -c1 -r - $@


.PHONY: clean
clean:
	rm -f plo.coff plo.efi *.o .*.d
