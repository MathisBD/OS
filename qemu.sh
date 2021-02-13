#!/bin/sh
set -e
. ./build.sh 

qemu-system-i386 -kernel "$SYSROOT$BOOTDIR/minios.kernel"