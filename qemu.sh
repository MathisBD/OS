#!/bin/sh
set -e
. ./iso.sh 

# -m is the memory in MB
qemu-system-i386 -cdrom minios.iso -m 4096