#!/bin/sh

set -e

echo "fetching submodules"
git submodule update

echo "Building libopencm3"
cd ext/libopencm3
make lib
cd -

echo "Building libgovernor"
mkdir -p ext/stage
cd ext/libgovernor
./autogen.sh
./configure --prefix=`pwd`/../stage --disable-shared --disable-check --target=arm-none-eabi --host=arm-none-eabi
make
make install
cd -