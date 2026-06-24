# FatFs R0.16 (Pub. Sept 13, 2025)
by ChaN, sourced from https://elm-chan.org/fsw/ff/  

(config informed by Coconut's FatFs https://github.com/ASU-SDSL/fatfs/tree/master)

## Additions and Config Log 

4/15/2026 - Adding CMakeLists.txt, based on Coconut's

Added `/flash/` and `/mram/` to start with testing a diskio built for flash memory 

SPI interactions need to be in critical sections to avoid context switches causing crashes or SPI corruption 

6/24/2026 - `diskio_test` works for flash