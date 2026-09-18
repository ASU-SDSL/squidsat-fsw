# FatFs R0.16 (Pub. Sept 13, 2025)
by ChaN, sourced from https://elm-chan.org/fsw/ff/  

## Overall Application Summary 
(config informed by Coconut's FatFs https://github.com/ASU-SDSL/fatfs/tree/master)

Changes from the original library should be limited to 
* `/flash`
* `/mram`
* `/test`
* `/source/ffconf.h`
* `/source/diskio.h` (only to add a `#include "ff.h"` for easier linking)

Changes are not platform agnostic and rely on FreeRTOS and Pico SDK functions. They are written to expect a FreeRTOS SMP on RP2350 with the Pico SDK. 

## Additions and Config Log 

4/15/2026 - Adding CMakeLists.txt, based on Coconut's

Added `/flash` and `/mram` to start with testing a diskio built for flash memory 

SPI interactions need to be in critical sections to avoid context switches causing crashes or SPI corruption 

6/24/2026 - `diskio_test` works for flash

6/24/2026 - started re-entrant and full filesystem using provided FreeRTOS functions in `/source/ffsystem.c`

6/24/2026 - working with filesystem test - still untested for re-entrant but that is implemented and was a default config so it'll likely work just fine. The SPI bus I don't think needs it's own thread protection so long as ONLY the filesystem uses that SPI bus. 