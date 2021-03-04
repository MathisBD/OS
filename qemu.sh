#!/bin/sh
set -e
. ./image.sh 

# -m is the memory in MB
qemu-system-i386 -hda $IMAGE -m 4096 -no-reboot -no-shutdown #-d int