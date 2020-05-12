#!/bin/bash
# Phoenix-RTOS
#
# plo - operating system loader
#
# Creates system image with plo bootloader
#
# Copyright 2001, 2005 Pawel Pisarczyk
# Copyright 2020 Phoenix Systems
# Author: Pawel Pisarczyk, Lukasz Kosinski
#
# This file is part of Phoenix-RTOS.

# This script creates image of Phoenix-RTOS kernel with plo bootloader for IA32 platform.
# Extra files passed in as optional parameters can be placed in the image.
# Usage:
# $1  - path to plo
# $2  - path to Phoenix-RTOS kernel ELF
# $3  - output path
# $4+ - optional extra files (paired with offset in KB, or 0 to add the file immediately after the previous one)
#       to be placed in the image , e.g. app1, app2, ext2 partition
# example: ./mkimg.sh _build/plo _build/phoenix-ia32-qemu.elf _boot/phoenix-ia32-qemu.img rootfs.ext2 1024

# Function adds new file to the $OUTPUT image (at the offset in KB, stored in offs variable)
# $1  - path to added file
function add2img {
	# Assume sector size = 512 bytes
	sector=`expr $offs \* 2`
	printf "Copying %s (%dKB, 0x%x)\n" $1 $offs $sector

	sz=`du -k $1 | awk '{ print $1 }'`
	dd if=$1 of=$OUTPUT seek=$offs bs=1024 >/dev/null 2>&1
	offs=`expr $offs + $sz + 1`
}

PLO=$1
shift

KERNEL=$1
shift

OUTPUT=$1
shift

# Remove last output
rm -f $OUTPUT

# Image starts with plo
cp $PLO $OUTPUT
sz=`du -k $PLO | awk '{ print $1 }'`
echo "Loader size: ${sz}KB"
echo "Adding padding after plo"
padsz=32
dd if=/dev/zero of=$OUTPUT seek=$sz bs=1024 count=$padsz >/dev/null 2>&1

# Add kernel at offset 32KB
offs=32
add2img $KERNEL

# Add extra files
while (( "$#" >= 2 )); do
	if (( $2 > $offs )); then
		offs=$2
	fi

	echo "Adding $1 at offset ${offs}KB to $OUTPUT"
	add2img $1

	shift 2
done
