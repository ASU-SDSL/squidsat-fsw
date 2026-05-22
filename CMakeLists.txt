cmake_minimum_required(VERSION 3.15)

# ── Platform Selection ────────────────────────────────────────────────────────
# Set defaults if not passed in from bash via -DPICO_BOARD and -DPICO_PLATFORM
if(NOT DEFINED PICO_BOARD)
    set(PICO_BOARD "pico2" CACHE STRING "Board type")
endif()

if(PICO_BOARD STREQUAL "pico")
    set(PICO_PLATFORM "rp2040" CACHE STRING "Pico platform")

else()
    set(PICO_PLATFORM "rp2350-arm-s" CACHE STRING "Pico platform")
endif()


message(STATUS "Building for board: ${PICO_BOARD}, platform: ${PICO_PLATFORM}")

# ── Pico SDK bootstrap ────────────────────────────────────────────────────────
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk" CACHE PATH "Path to pico-sdk")
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
# ── Application ───────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/main/main.c

    # drivers 
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/src/i2c_util.c
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/src/rtc.c

    # utilities 
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/src/timing.c
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/src/log.c


    # tasks 
    ${CMAKE_CURRENT_LIST_DIR}/main/tasks/gse/gse.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}/main/drivers/include
    ${CMAKE_CURRENT_LIST_DIR}/main/utilities/include
    
    ${CMAKE_CURRENT_SOURCE_DIR}/main/tasks
    ${CMAKE_CURRENT_SOURCE_DIR}/main/tasks/gse
    ${CMAKE_CURRENT_SOURCE_DIR}/main/utilities/src
)

# add FatFs library 
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/ff16 build)

target_link_libraries(${PROJECT_NAME}
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    pico_stdlib
    pico_multicore
    pico_aon_timer
    hardware_gpio
    hardware_i2c

    FatFs
)
#pico_set_binary_type(${PROJECT_NAME} no_flash)

pico_enable_stdio_usb(${PROJECT_NAME}  1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})