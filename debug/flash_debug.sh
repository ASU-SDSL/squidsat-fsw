#!/bin/bash

#build in debug 
./build.sh -d

# Check if we build successfully
if [ $? != 0 ]; then
    echo >&2 "\033[0;31m \nFailed to build!"
    exit 1
fi

echo "Flashing target device..."

# flash 
openocd -f interface/cmsis-dap.cfg \
    -f debug/rp2350.cfg \
    -c "adapter speed 1000" \
    -s tcl \
    -c "program ./build/freertos_rp2350.elf reset exit"
    
echo "Done."