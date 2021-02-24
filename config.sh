#!/bin/sh

# will probably add libc here in the future
export PROJECTS="libc kernel user"

# sysroot file structure
export SYSROOT="$(pwd)/sysroot"
export BOOTDIR=/boot 
export INCLUDEDIR=/usr/include
export LIBDIR=/usr/lib

# compiler and compiler options
export CC=i686-elf-gcc
export AR=i686-elf-ar
export CC="$CC --sysroot=$SYSROOT"
export CFLAGS="-O0 -g"
export CPPFLAGS=""
# the cross compiler was configured with
# --without-headers but not --with-sysroot
# and -elf gcc targets don't have a system include directory
# so we need to do this ?? or not ?
# export CC="$CC -isystem $INCLUDEDIR"


