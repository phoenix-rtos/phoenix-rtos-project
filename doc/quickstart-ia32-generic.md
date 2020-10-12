# ia32-generic
This version is designated for generic PC based on IA32 processor. To launch this version the final disk image should be downloaded. The image is created as the final artifact of project building and is located in _boot directory. The image consist of bootloader (plo), kernel, TTY VGA driver, ATA driver with ext2 filesystem.

The disk image can be downloaded from 

## Running image under qemu
To run image under qemu you should type:

qemu-system-i386 -hda _boot/phoenix-ia32-generic.disk 

<img src="qemu-ia32-generic.png" width="600px">

## Running image on regular hardware


