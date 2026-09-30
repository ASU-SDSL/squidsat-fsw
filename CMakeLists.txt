cmake_minimum_required(VERSION 3.15)

# metadata export for VSCode linter
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Platform Selection ────────────────────────────────────────────────────────
# Set defaults if not passed in from bash via -DPICO_BOARD and -DPICO_PLATFORM
if(NOT DEFINED PICO_BOARD)
    set(PICO_BOARD "pico2" CACHE STRING "Board type")
endif()

if(PICO_BOARD STREQUAL "pico")
    set(PICO_PLATFORM "rp2040" CACHE STRING "Pico platform")

else()
    set(PICO_PLATFORM "rp2350" CACHE STRING "Pico platform")
endif()


message(STATUS "Building for board: ${PICO_BOARD}, platform: ${PICO_PLATFORM}")

# ── Pico SDK Define ────────────────────────────────────────────────────────
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk" CACHE PATH "Path to pico-sdk")

# ── CMake Toolchain for Pico SDK ──────────────────────────────────────────────
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(PICO_PLATFORM STREQUAL "rp2040")
        set(CMAKE_TOOLCHAIN_FILE 
            ${PICO_SDK_PATH}/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake
            CACHE FILEPATH "Toolchain file")
    else()
        set(CMAKE_TOOLCHAIN_FILE 
            ${PICO_SDK_PATH}/cmake/preload/toolchains/pico_arm_cortex_m33_gcc.cmake
            CACHE FILEPATH "Toolchain file")
    endif()
endif()

# ── Pico SDK bootstrap ────────────────────────────────────────────────────────
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# ── FreeRTOS kernel ───────────────────────────────────────────────────────────
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel" CACHE PATH "Path to FreeRTOS-Kernel")

if(PICO_PLATFORM STREQUAL "rp2040")
    include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
else()
    include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)
endif()

# ── Project ───────────────────────────────────────────────────────────────────
# Name the project based on the target so .uf2 filenames are predictable
if(PICO_PLATFORM STREQUAL "rp2040")
    project(freertos_rp2040 C CXX ASM)
else()
    project(freertos_rp2350 C CXX ASM)
endif()

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Config debug flags - recommended by raspberry pi 
set(PICO_DEOPTIMIZED_DEBUG 1)

# how coconut did it 
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# ── Application ───────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/main/main.c

    # drivers 
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/src/i2c_util.c
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/src/rtc.c
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/src/mcp2515.c

    # utilities 
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/src/timing.c
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/src/log.c
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/src/filesystem.c

    # tasks 
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/gse/gse.c
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/steve/steve.c
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/steve/jobs/sensor_job/sensor_job.c
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/steve/jobs/gse_test/gse_test.c


    # usb 
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/usb_serial/usb_serial.c
    # modified stuff we still want from the pico sdk for usb
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/usb_serial/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/usb_serial/picotool_interface.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/include
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/include
    ${CMAKE_CURRENT_SOURCE_DIR}/main/utilities/src
    
    ${CMAKE_CURRENT_SOURCE_DIR}/main/tasks
    ${CMAKE_CURRENT_SOURCE_DIR}/main/tasks/gse
    ${CMAKE_CURRENT_SOURCE_DIR}/main/tasks/usb_serial
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/steve
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/steve/jobs

)

# add FatFs library 
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/ff16 build)

target_link_libraries(${PROJECT_NAME}
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    tinyusb_device
    tinyusb_board

    pico_stdlib
    pico_unique_id
    pico_usb_reset_interface

    pico_multicore
    pico_aon_timer
    hardware_gpio
    hardware_i2c
    hardware_spi

    FatFs
)

# config debug mode macro flag
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
)

#pico_set_binary_type(${PROJECT_NAME} no_flash)

pico_enable_stdio_usb(${PROJECT_NAME}  1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})