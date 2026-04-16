# SQUIDSAT FSW 

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

# NOTE: PUT IN README THAT YOU NEED TO CLEAR BUILD FOLDER IF YOU WANT TO TEST BETWEEN RP2350 AND RP2040 BACK TO BACK
# RUN cd build && rm -rf * and then run ./build.sh -p (handle)