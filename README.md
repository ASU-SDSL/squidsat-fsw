# FreeRTOS SMP RP2350 - SQUIDSAT FSW 

## Clone

After cloning the repo, initialize the submodules:

```bash
git submodule update --init --recursive
```

## Build

Create a build directory, configure with CMake, and build:

```bash
mkdir build
cd build
cmake .. -DPICO_PLATFORM=rp2350
make
```
