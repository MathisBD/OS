#!/bin/sh

# projects are compiled/installed in this order
# therefore libc must appear before kernel
export PROJECTS="bootloader libc kernel user"

# executables
export KERNEL="minios.elf" # elf file 
export FIRST_STAGE="first_stage.bin" # flat binary
export SECOND_STAGE="second_stage.bin" # flat binary

# sysroot file structure
# (the makefiles install their headers/binaries here)
export SYSROOT="$(pwd)/sysroot"
export BOOTDIR=/boot 
export INCLUDEDIR=/usr/include
export LIBDIR=/usr/lib

# image file structure
# (the root of the initial miniOS filesystem)
export IMAGE="hdd_minios.img"
export IMGDIR="$(pwd)/imgdir"

# C compiler and compiler options
export CC=i686-elf-gcc 
export AR=i686-elf-ar
export CFLAGS="--sysroot=$SYSROOT -g \
-Wno-unused-parameter -Wno-unused-label -Wno-int-conversion \
-Wno-incompatible-pointer-types -Werror=implicit-function-declaration"
export CPPFLAGS=""
# the cross compiler was configured with
# --without-headers but not --with-sysroot
# and -elf gcc targets don't have a system include directory
# so we need to do this ?? or not ?
# export CC="$CC -isystem $INCLUDEDIR"


