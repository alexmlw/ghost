#!/bin/bash

. "./ghost.sh"

if [ -f $SYSROOT/system/lib/libcairo.a ]; then
    echo "Skipping build of ports"
else
    echo "Building ports"
    pushd patches/ports
    $SH port.sh zlib/1.2.8
    $SH port.sh pixman/0.32.6
    $SH port.sh libpng/1.6.18
    $SH port.sh freetype/2.5.3
    $SH port.sh cairo/1.12.18
    popd
    echo ""
fi
