# SquidSat OBC Flight Software

## Clone

After cloning the repo, initialize the submodules:

```bash
git submodule update --init --recursive
```

## Build

Use `build.sh` to create/configure the `build/` directory and compile the
firmware for the Pico version you want:

```bash
./build.sh -p pico
```

or, for Pico 2:

```bash
./build.sh -p pico2
```

The `-p` flag selects the target board:

- `pico` builds for RP2040.
- `pico2` builds for RP2350.

To build in debug mode, add `-d`:

```bash
./build.sh -p pico2 -d
```

## Deploy

After the build finishes, flash the Pico with:

```bash
./deploy.sh
```

The first time you flash a pico, you will have to do the traditional drag and drop
the uf2 file. This is due to the fact that we need to get usb initialized on the pico.

## Onboarding Tips

Every subdirectory under main has a README pertaining to said subdirectory which is where 
your first questions can likely be answered. You should spend some time reading around the codebase
to understand the general layout and interactions before trying to implement code right away. Depending on first task, do not try to understand everything at once but focus on what surrounds your task and gain a foothold. A good resource for developing with pico can be found [here](https://pip-assets.raspberrypi.com/categories/609-microcontroller-boards/documents/RP-009085-KB-4-raspberry-pi-pico-c-sdk.pdf), this document is huge so I'd recommend using ctrl+f to try and find what you need, or google your question and then ctrl+f whatever you find there. The SDSL software onboarding drive folder itself can be found [here](https://drive.google.com/drive/folders/1eDmDWeWhAOqVWXUOmBbYNyWdQSxA6mLc?usp=drive_link). In general, it is always best to ask other SDSL members as they have more domain knowledge and can likely answer if not guide you to where to find the answer. 

<hr />

<a href="https://asu-sdsl.github.io/" target="_blank"><img src="https://asu-sdsl.github.io/images/sdsl-logo.png" alt="SDSL Logo" style="height:50px; display:block; margin: 0 auto;"></a>