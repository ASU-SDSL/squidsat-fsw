#!/bin/bash
BUILD_DIR=build ;
build_path=./build ;
PICO_VERSION="pico2" ;
PICO_FILE="build/freertos_rp2040.uf2" ;
PICO2_FILE="build/freertos_rp2350.uf2" ;
DEBUG_MODE=0

# ------ Flag Catching ------

while getopts "p:d" opt; do
    case $opt in
        p) PICO_VERSION="$OPTARG"
           echo "$PICO_VERSION" ;;
        d) DEBUG_MODE=1
           echo "Debug Build" ;; # Need to get this working
    esac
done

# ------ Set Debug --------------
if (( DEBUG_MODE == 1 )); then 
    BUILD_TYPE="Debug"
else 
    BUILD_TYPE="Release"
fi

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
    -DPICO_PLATFORM=rp2040 \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

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
    -DPICO_PLATFORM=rp2350 \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

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

# ------ Size Report ------
if [ "$PICO_VERSION" == "pico" ]; then
    ELF_FILE="$BUILD_DIR/freertos_rp2040.elf"
    FLASH_SIZE=$((2 * 1024 * 1024))   # Pico: 2 MB
    RAM_SIZE=$((264 * 1024))
else
    ELF_FILE="$BUILD_DIR/freertos_rp2350.elf"
    FLASH_SIZE=$((4 * 1024 * 1024))   # 32 Mbit MRAM: 4 MB
    RAM_SIZE=$((520 * 1024))
fi

if [ -f "$ELF_FILE" ]; then
    echo "------ Size Report ------"
    arm-none-eabi-size "$ELF_FILE" | awk -v fs="$FLASH_SIZE" -v rs="$RAM_SIZE" 'NR==2 {
        flash = $1 + $2; ram = $2 + $3
        printf "Flash: %7d bytes (%6.1f KB)  %5.2f%% of %d KB\n", flash, flash/1024, 100*flash/fs, fs/1024
        printf "RAM:   %7d bytes (%6.1f KB)  %5.2f%% of %d KB\n", ram,   ram/1024,   100*ram/rs,   rs/1024
    }'
else
    echo "No ELF found at $ELF_FILE, skipping size report"
fi