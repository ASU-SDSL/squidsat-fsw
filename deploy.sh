#!/bin/bash
PICO_FILE="build/freertos_rp2040.uf2"
PICO2_FILE="build/freertos_rp2350.uf2"

# ------ Reboot Pico into BOOTSEL ------
picotool reboot -f -u 2>/dev/null
sleep 3

# ------ Deploying ------
if [ -f "$PICO_FILE" ]; then
    if picotool load "$PICO_FILE" -f --verify; then
        echo "Deployment successful and verified for RP2040!"
        picotool reboot
        picotool info -a
    else
        echo "Deployment failed or verification mismatch!"
        exit 1
    fi



elif [ -f "$PICO2_FILE" ]; then
    if picotool load "$PICO2_FILE" -f --verify; then
        echo "Deployment successful and verified for RP2350!"
        picotool reboot
        picotool info -a
    else
        echo "Deployment failed or verification mismatch!"
        exit 1
    fi

else
    echo "No .uf2 file found in build/"
    exit 1
fi

# For Aidan to be able to use WSL more smoothly when testing code. Place at top of pico2 elif
# GREEN=$(tput setaf 2) # Text colors just for visual appeal
# CYAN=$(tput setaf 6)
# YELLOW=$(tput setaf 3)
# RED=$(tput setaf 1)
# RESET=$(tput sgr0)


# if  lsusb -d "2e8a:0009" > /dev/null ; then                 #Standard Mode VID:PID Both change per physical PI board
#     picotool reboot -f -u
# fi

# until lsusb -d 2e8a:000f; do                        
#     echo "${YELLOW}Waiting for Pico to attach to WSL in BOOTSEL mode...${RESET}" #Bootsel Mode VID:PID 
#     sleep 1
# done