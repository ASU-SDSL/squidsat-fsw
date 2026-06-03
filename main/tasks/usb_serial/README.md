# TinyUSB with FreeRTOS SMP
This is a dump overview of the stuff that went into getting this to work to make sure it doesn't get forgotten

## The Problem
The root of the problem is that although Raspberry Pi and the Pico SDK are set up to be somewhat compatible with FreeRTOS SMP, TinyUSB just isn't, it's only setup to be compatible with non-SMP FreeRTOS. The TinyUSB stack can call the callbacks to get it to work, if they are left as interrupts at anytime with no protection for SMP. This means that we can't just use the TinyUSB FreeRTOS config settting, but instead need to use the Pico config setting and do our own port into FreeRTOS SMP. This is once of the reasons the TinyUSB backend in any RTOS is often it's own task. 

## Moving the TinyUSB execution 
The first thing to fix the problems to disable the Pico SDK's USB stack and use the `tiny_usb` and `tinyusb_board` libraries directly. Also to define our own `tusb_config.h`. This then allowed for `tusb_init()` and `tusb_task()` to be called in a dedicated FreeRTOS task (tied to Core 0) with mutex protection on every TinyUSB call after tusb_init(). (see Arduino-Pico's implementation for reference because a lot of the stack size and other decisions are ripped directly from it). 

The `usb_descriptors.c` to get this to work - which is really just TinyUSB callbacks and definitions to identify the device is mostly the same as the default Pico SDK one but without the pico/stdio_usb.h include. I change it a bit more later for Picotool so the final version is fairly different. 

USB CDC is the serial input/output, and with this setup calls to `tud_cdc...` start to work properly again. 

**This does fix the crashing on output problem but doesn't link the new updated TinyUSB to stdio**

## Fixing USB stdio 
Without using the Pico SDK stdio init, printf and other stdio don't do anything because there are no registered drivers for them to call. Turns out the way the Pico SDK links stdio to hardware is by having a list of registered drivers that it uses for input and output calls. To get stdio to use our new drivers again we need to use `stdio_set_driver_enabled` which the Pico SDK says shouldn't be considered stable, but is in pico-playground, so it should be fine but **should be considered carefully for any upgrade of the used SDK**. I think they are going to keep supporting it but I couldn't find much official statement on it. Regardless this function registers a new driver with the stdio which connects the normal C stdio with the new, core-safe, driver implementation. 

**At this point stdio works and everything for usb input/output should work other than Picotool**

**Any stdio functions and any USB functions now don't work and shouldn't be used until after the USB Task is started.**

## Fixing Picotool 
**This part diverges a decent bit from the SDK so any updates to the Pico SDK or Picotool might break it, but probably not** 
To fix Picotool the preprocessor conditional parts of `usb_descriptors.c` needed to get mangled because I didn't want to risk pulling in code unexpectedly by using the built-in macros. This means with this setup Picotool can't be turned off. But all that really needed to get done was making sure any part of the code that was only compiled if `PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE` was set was compiled. The exception here is `.bcdUSB = 0x0210` because that's only for Windows compatibility and I didn't want to do the other stuff for that - **with this setup Windows systems can't use Picotool**.

The big part of this is in `picotool-interface.c` which is selected functions and definitions from the Pico SDK's `stdio_usb.h` and `reset_interface.c` to avoid pulling in extra stuff from the Pico SDK's USB backend and stuff that we don't need for this project. It essentially creates a new USB interface driver for the reset and registers it with TinyUSB.  

**With all that it should work for everything we are using it for. Although a caveat is that the reset interface implementation is just what is needed to get the current `deploy.sh` to work so it may not be compatible with every Picotool command.**

## Good Resources for this I referenced 
Using `lsusb` to get info on the USB interface 

General CDC and TinyUSB Stuff:
* https://docs.tinyusb.org/en/latest/index.html (not super helpful though, mostly for usb concepts) 
* https://github.com/hathach/tinyusb/tree/bbdb41995de6510b837ad239933e1823ca175314/src/class/cdc

Pico STDIO drivers
* https://github.com/raspberrypi/pico-playground/blob/master/stdio/pio/stdio_pio.c
* https://www.raspberrypi.com/documentation/pico-sdk/runtime.html#group_pico_stdio

Arduino-Pico's USB implementation 
* https://github.com/earlephilhower/arduino-pico/blob/master/cores/rp2040/USB.cpp#L76

The Pico SDK's Docs are OK - most stuff is from pico_stdio_usb 
* https://github.com/raspberrypi/pico-sdk/tree/a1438dff1d38bd9c65dbd693f0e5db4b9ae91779/src/rp2_common/pico_stdio_usb