#!/bin/bash
BUILD_DIR=build ;
build_path=./build ;
PICO_VERSION="pico2" ;
PICO_FILE="build/freertos_rp2040.uf2" ;
PICO2_FILE="build/freertos_rp2350.uf2" ;

# ------ Flag Catching ------

while getopts "p:d" opt; do
    case $opt in
        p) PICO_VERSION="$OPTARG"
           echo "$PICO_VERSION" ;;
        d) echo "Debug Build working" ;; # Need to get this working
    esac
done



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

# ------ Build Folder File Check ------

if [ -f "$PICO2_FILE" ] && [ "$PICO_VERSION" == "pico" ]; then
    echo "Trying to build for RP2040 but have RP2350 uf2"
    echo "Deleting files in ./build..."
    rm -rf "$build_path"/*
elif [ -f "$PICO_FILE" ] && [ "$PICO_VERSION" == "pico2" ]; then
    echo "Trying to build for RP2350 but have RP2040 uf2"
    echo "Deleting files in ./build..."
    rm -rf "$build_path"/*
fi

# ------ Building ------

echo "Attempting to build"

if [ "$PICO_VERSION" == "pico" ]; then
    echo "Building for RP2040..."

    cmake -S . -B "$BUILD_DIR" \
    -DPICO_BOARD=pico \
    -DPICO_PLATFORM=rp2040

    err=$?
    if [ $err -ne 0 ]; then
        echo "CMake failed!"
        exit $err
    fi
    make -C "$BUILD_DIR"
    err=$?
    if [ $err -ne 0 ]; then
        echo "Make failed!"
        exit $err
    fi
elif [ "$PICO_VERSION" == "pico2" ]; then
    echo "Building for RP2350..."

    cmake -S . -B "$BUILD_DIR" \
    -DPICO_BOARD=pico2 \
    -DPICO_PLATFORM=rp2350-arm-s
    err=$?
    if [ $err -ne 0 ]; then
        echo "CMake failed!"
        exit $err
    fi
    make -C "$BUILD_DIR"
    err=$?
    if [ $err -ne 0 ]; then
        echo "Make failed!"
        exit $err
    fi

else
    echo "Unknown microcontroller"
    echo "Aborting..."
    exit 1
fi

# ------ Deploying ------

if [ "$PICO_VERSION" == "pico" ]; then
    picotool load -x "$PICO_FILE" -f
elif [ "$PICO_VERSION" == "pico2" ]; then
    picotool load -x "$PICO2_FILE" -f
fi