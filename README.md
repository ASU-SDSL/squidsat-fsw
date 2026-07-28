# SQUIDSAT FSW 

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