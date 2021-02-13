#!/bin/bash
set -e
. ./config.sh

# install all the headers in sysroot, 
# so that gcc can find them when compiling
mkdir -p "$SYSROOT"

for PROJECT in $PROJECTS; do 
    (cd $PROJECT && make install-headers)
done