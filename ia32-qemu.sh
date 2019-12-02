qemu-system-i386 -serial stdio -netdev bridge,br=virbr0,id=net0 -device rtl8139,netdev=net0,id=nic0,addr=03.0 -hda phoenix-ia32.ext2
