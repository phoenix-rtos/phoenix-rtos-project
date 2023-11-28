IMG_ROOTFS="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv7a9-zynq7000-qemu/rootfs.jffs2"
IMG_SDCARD="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv7a9-zynq7000-qemu/sdcard.img"
dd if=/dev/zero of="$IMG_SDCARD" bs=512 count=65536
fdisk -u ${IMG_SDCARD} << EOF
o
n
p
1
2048
+12M
w
q
EOF
dd conv=notrunc if="$IMG_ROOTFS" of="$IMG_SDCARD" bs=512 seek=2048
