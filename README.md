# SquidSat OBC Flight Software

## Clone

After cloning the repo, initialize the submodules:

```bash
git submodule update --init --recursive
```

## Python Virtual Environment (`.venv`)

The build generates the C protobuf code for CAN messages (`link.pb.c` / `link.pb.h` from `main/tasks/can/link.proto`) with the nanopb generator, which is a Python script. To keep its Python packages separate from your system Python, `build.sh` runs it from a project-local virtual environment in `.venv/`.

You do not need to create or activate it yourself. On every run, `build.sh`:

1. Checks that the nanopb submodule is present (`lib/nanopb/requirements.txt`). If it is missing, it stops and tells you to run `git submodule update --init`.
2. Creates `.venv/` with `python3 -m venv .venv` if it does not exist yet.
3. Checks that the generator's packages import (`google.protobuf`, `grpc_tools`). If not, it installs `lib/nanopb/requirements.txt` (`protobuf` and `grpcio-tools`) into the venv.
4. Passes `.venv/bin/python3` to CMake with `-DPython3_EXECUTABLE`, so the generator always runs with the venv's Python.

The first build after cloning therefore takes a little longer while the venv is created and the packages download. Later builds skip both steps.

**Requirements:** `python3` must be on your `PATH` with the `venv` module available. On Debian/Ubuntu, install it with `sudo apt install python3-venv`. macOS's Python already includes it.

**Notes:**

- `.venv/` is listed in `.gitignore`. Never commit it; each machine creates its own.
- If you run `cmake` yourself instead of `build.sh`, pass the venv's Python, e.g. `-DPython3_EXECUTABLE=$PWD/.venv/bin/python3`, or CMake will use your system Python, which may not have the packages.
- If the venv breaks (for example after upgrading Python), delete it with `rm -rf .venv` and run `build.sh` again to recreate it.

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