#!/bin/bash

PICO_FILE="build/freertos_rp2040.uf2"
PICO2_FILE="build/freertos_rp2350.uf2"

# ------ Reboot Pico into BOOTSEL ------
picotool reboot -f -u 2>/dev/null
sleep 3

# ------ Deploying ------
if [ -f "$PICO_FILE" ]; then
    until lsusb -d 2e8a:0003; do                    #BOOTSEL Mode VID:PID     
        echo "Waiting for Pico in BOOTSEL mode..."
        sleep 1
    done
    if picotool load "$PICO_FILE" -f --verify; then
        echo "Deployment successful and verified for RP2040!"
        picotool reboot
    else
        echo "Deployment failed or verification mismatch!"
        exit 1
    fi

elif [ -f "$PICO2_FILE" ]; then
    if picotool load "$PICO2_FILE" -f --verify; then
        echo "Deployment successful and verified for RP2350!"
        picotool reboot
    else
        echo "Deployment failed or verification mismatch!"
        exit 1
    fi

else
    echo "No .uf2 file found in build/"
    exit 1
fi