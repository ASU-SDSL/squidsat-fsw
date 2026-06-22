add_library(FatFs INTERFACE)

set(USING_FLASH 1)

IF (USING_FLASH)
    set(DISKIO_SOURCE
        ${CMAKE_CURRENT_LIST_DIR}/flash/diskio.c
        ${CMAKE_CURRENT_LIST_DIR}/flash/flash.c
    )

    set(DISKIO_INCLUDE
        ${CMAKE_CURRENT_LIST_DIR}/flash
    )
ELSE()
    set(DISKIO_SOURCE
        ${CMAKE_CURRENT_LIST_DIR}/mram/diskio.c
        ${CMAKE_CURRENT_LIST_DIR}/mram/mram.c
    )

    set(DISKIO_INCLUDE
        ${CMAKE_CURRENT_LIST_DIR}/mram
    )
ENDIF()

target_sources(FatFs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/source/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ffunicode.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ff.c
    ${CMAKE_CURRENT_LIST_DIR}/test/diskio_test.c
    
    # custom implemented diskio
    ${DISKIO_SOURCE}
)

target_include_directories(FatFs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/source
    ${CMAKE_CURRENT_LIST_DIR}/test

    # custom implemented diskio
    ${DISKIO_INCLUDE}
)