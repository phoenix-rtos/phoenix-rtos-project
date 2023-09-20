(gdb) target remote localhost:1234
Remote debugging using localhost:1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x00008d36 in ?? ()
file _build/armv7a9-zynq7000-qemu/prog/phoenix-armv7a9-zynq7000.elf
target remote localhost:1234
b phoenix-rtos-kernel/hal/armv7a/exceptions.c:100
c
