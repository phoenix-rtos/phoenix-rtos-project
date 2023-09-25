file _build/armv7a9-zynq7000-qemu/prog/phoenix-armv7a9-zynq7000.elf
b /github/workspace/phoenix-rtos-kernel/hal/armv7a/exceptions.c:52
# b _threads_schedule if context == 0xc014ecf8 && context->pc > 0xc0011cb4
target remote localhost:1234
b foobar
c
# b /github/workspace/phoenix-rtos-kernel/proc/process.c:1502
