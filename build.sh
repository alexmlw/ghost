#!/bin/bash
source ghost.sh

pushd() {
    command pushd "$@" > /dev/null
}

popd() {
    command popd "$@" > /dev/null
}

if [ ! -d $SYSGHOST ]; then
    echo "Create ghost dir"
    mkdir $SYSGHOST
    cp -r $SYSROOT/* $SYSGHOST
    cp -r ../sysroot/* $SYSGHOST
fi

# Prepare C++ library
echo "Building libuser"
pushd libuser
$SH build.sh clean
$SH build.sh all
popd


# Make all applications
pushd applications
for dir in */; do
	echo "Building $dir"
	pushd $dir
		$SH build.sh clean
		$SH build.sh all
	popd
	echo ""
done
popd


# Finally build kernel
echo "Building kernel"
pushd kernel
$SH build.sh clean
$SH build.sh all
popd
echo ""
