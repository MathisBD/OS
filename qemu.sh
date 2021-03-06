#!/bin/sh
set -e
. ./image.sh 

# -m is the memory in MB
qemu-system-i386 \
-drive file=$IMAGE,format=raw,index=0,media=disk \
-m 4096 -no-reboot -no-shutdown #-d int