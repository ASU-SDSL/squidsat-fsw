#!/bin/bash

PICO_FILE="build/freertos_rp2040.uf2" ;
PICO2_FILE="build/freertos_rp2350.uf2" ;

# ------ Reboot Pico ------
picotool reboot -f -u
sleep 2 # this is just to make sure it doesn't miss the build

# ------ Deploying ------

if [ -f "$PICO_FILE" ]; then
    picotool load -x "$PICO_FILE" -f
elif [ -f "$PICO2_FILE" ]; then
    picotool load -x "$PICO2_FILE" -f
else
    echo "No .uf2 file found in build/"
    exit 1
fi

echo "Deployment Complete!"