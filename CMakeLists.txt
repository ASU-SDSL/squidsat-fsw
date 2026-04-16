cmake_minimum_required(VERSION 3.15)

# ── Must be set FIRST — before any include() ──────────────────────────────────
# Tells the SDK and the FreeRTOS import script which chip/core we're targeting.
set(PICO_PLATFORM "rp2350-arm-s" CACHE STRING "Pico platform")
set(PICO_BOARD    "pico2"        CACHE STRING "Board type")

# ── Pico SDK bootstrap ────────────────────────────────────────────────────────
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk" CACHE PATH "Path to pico-sdk")
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# ── FreeRTOS kernel — include the import script directly from the submodule ───
# No need to copy anything — the file already exists inside FreeRTOS-Kernel.
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel" CACHE PATH "Path to FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)

# ── Project ───────────────────────────────────────────────────────────────────
project(freertos_rp2350 C CXX ASM)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

# Initialise the Pico SDK (must come after project())
pico_sdk_init()

# ── Application ───────────────────────────────────────────────────────────────
add_executable(freertos_rp2350
    main/main.c

    # drivers 
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/src/rtc.c

    # tasks 
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/gse/gse.c
)

target_include_directories(freertos_rp2350 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/include 
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/include 

    # tasks
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/gse
)

target_link_libraries(freertos_rp2350
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    pico_stdlib
    pico_multicore
    hardware_gpio
)

pico_enable_stdio_usb(freertos_rp2350  1)
pico_enable_stdio_uart(freertos_rp2350 0)

pico_add_extra_outputs(freertos_rp2350)