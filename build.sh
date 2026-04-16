#!/bin/bash
build_path="./build"
timeout=15
count=0
BUILD_DIR=build

# ------ Reboot Pico ------
picotool reboot -f -u

# ------ Build Folder Check ------

if [ ! -d $BUILD_DIR ]; then
    echo "Build Dir doesn't exist"
    echo "Making Build Dir ..."
    mkdir -p $BUILD_DIR
else
    echo "Build Dir exists ..."
fi
# ------ Building ------

echo "Attempting to build"

cmake -S . -B "$BUILD_DIR"
    err=$?

if [[ $err -ne 0 ]]; then
    echo "Building Failed"
    exit 1
fi

# ------ Deploying ------

picotool load -x ${build_path}/freertos_rp2350.uf2 -f