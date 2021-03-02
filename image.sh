#!/bin/sh
set -e
. ./build.sh 

# make the initial disk image
DISK_BLOCK_SIZE=2048
DISK_BLOCK_COUNT=32K

mkdir -p $IMGDIR
mkdir -p $IMGDIR$BOOTDIR

cp -r $SYSROOT$BOOTDIR/. $IMGDIR/$BOOTDIR/.

# zeroed out disk
dd if=/dev/zero of=./$IMAGE bs=$DISK_BLOCK_SIZE count=$DISK_BLOCK_COUNT

LOOP_DEV=$(sudo losetup -Pf --show ./$IMAGE)
# format disk
sudo mkfs.ext2 $LOOP_DEV -b $DISK_BLOCK_SIZE -d $IMGDIR
# copy first stage bootloader
# we have to do this through a loop device,
# otherwise it cuts the end of the file (after 512 bytes)
sudo dd if=$SYSROOT$BOOTDIR/$FIRST_STAGE of=$LOOP_DEV bs=512 count=1
sudo losetup -d $LOOP_DEV

