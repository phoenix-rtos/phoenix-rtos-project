#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU (ia32-generic)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

DIR="$(dirname "$BASH_SOURCE")"                          # Script directory
DRIVES="-hda $DIR/../../_boot/phoenix-ia32-generic.disk" # Default drives
GPUS="-vga std"                                          # Default graphics adapters
NETDEVS=""                                               # Default network devices
SYSTEM="-serial stdio"                                   # Default system options

# Device counters (used for generating device ID)
ata=0
vblk=0
net=0

# Parse args
while [ "$#" -gt 0 ]; do
	case "$1" in
		# Print help message
		-help|-h)
			echo "Usage: $0 [options] or no args to run system image in default configuration"
			echo -e "\t-ata|-ide <disk path>             - add ATA drive with given disk image"
			echo -e "\t-virtio-blk|-vblk <disk path>     - add VirtIO block device with given disk image"
			echo -e "\t-virtio-gpu|-vgpu [outputs]       - add VirtIO GPU device with 'outputs' connected displays"
			echo -e "\t-virtio-net|-vnet <backend> [nic] - add VirtIO network device connected to given network backend"
			echo -e "\t-help|-h                          - print help message"
			echo -e "\tanything else                     - pass given option directly to QEMU startup command"
			exit 1
			;;
		# Add ATA drive
		-ata|-ide)
			if [ -z "$2" ]; then
				echo "$1: missing disk path"
				exit 1
			fi
			DRIVES+=" -drive file=$2,if=none,id=ata$ata -device ide-hd,drive=ata$ata"
			((ata++))
			shift 2
			;;
		# Add VirtIO drive
		-virtio-blk|-vblk)
			if [ -z "$2" ]; then
				echo "$1: missing disk path"
				exit 1
			fi
			DRIVES+=" -drive file=$2,cache=unsafe,if=none,id=vblk$vblk -device virtio-blk-pci,drive=vblk$vblk"
			((vblk++))
			shift 2
			;;
		# Add VirtIO GPU device
		-virtio-gpu|-vgpu)
			GPUS+=" -device virtio-gpu-pci"
			if [ -n "$2" ] && [ "$2" -eq "$2" ] 2> /dev/null; then
				GPUS+=",max_outputs=$2"
				shift
			fi
			shift
			;;
		# Add VirtIO network device
		-virtio-net|-vnet)
			if [ -z "$2" ]; then
				echo "$1: missing backend"
				exit 1
			fi
			NETDEVS+=" -netdev $2,id=net$net -device virtio-net-pci,netdev=net$net,id=nic$net"
			if [ ! -z "$3" ] && case "$3" in -*) false;; esac; then
				NETDEVS+=",$3"
				shift
			fi
			((net++))
			shift 2
			;;
		# Add Realtek RTL8139 network card
		-rtl8139|-rtl)
			if [ -z "$2" ]; then
				echo "$1: missing backend"
				exit 1
			fi
			NETDEVS+=" -netdev $2,id=net$net -device rtl8139,netdev=net$net,id=nic$net"
			if [ ! -z "$3" ] && case "$3" in -*) false;; esac; then
				NETDEVS+=",$3"
				shift
			fi
			((net++))
			shift 2
			;;
		# Pass other options directly to QEMU startup command
		*)
			SYSTEM+=" $1"
			shift
			;;
	esac
done

# Print and execute QEMU command
echo "qemu-system-i386 $DRIVES $GPUS $NETDEVS $SYSTEM" && exec qemu-system-i386 $DRIVES $GPUS $NETDEVS $SYSTEM
