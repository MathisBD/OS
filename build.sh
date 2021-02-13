#!/bin/sh
set -e
. ./config.sh

# first install headers in the sysroot, 
# otherwise gcc will not find them and won't compile
. ./headers.sh

for PROJECT in $PROJECTS; do 
    (cd $PROJECT && make install)
done