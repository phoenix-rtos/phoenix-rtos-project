#!/bin/sh
cd ..
qemu-system-aarch64 -machine virt,gic-version=2 -cpu cortex-a53,pmu=off,aarch64=on -kernel _build/aarch64a53-zynqmp-virt/prog.stripped/plo-aarch64a53-zynqmp.elf -smp 1 -device loader,addr=0xff5e023c,data=0x80008fde,data-len=4 -device loader,addr=0xff9a0000,data=0x80000218,data-len=4  -serial mon:stdio -accel kvm -device loader,addr=0x49000000,file=_boot/aarch64a53-zynqmp-virt/flash0.disk -s -m 600M
