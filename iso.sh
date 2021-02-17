#!/bin/sh
set -e
. ./build.sh 

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp "$SYSROOT$BOOTDIR/minios.kernel" isodir/boot/minios.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "minios" {
    multiboot /boot/minios.kernel
}
EOF
grub-mkrescue -o minios.iso isodir