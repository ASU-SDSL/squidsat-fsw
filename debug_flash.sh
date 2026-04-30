#!/bin/bash

./build.sh

echo "Flashing target device..."

sudo openocd -f interface/cmsis-dap.cfg \
    -f debug/rp2350.cfg \
    -c "adapter speed 5000" \
    -c "program build/debug/freertos_rp2350.elf verify reset exit"

echo "Done." 