add_library(FatFs INTERFACE)

set(USING_FLASH 1)

IF (USING_FLASH)
    set(DISKIO_SOURCE
        ${CMAKE_CURRENT_LIST_DIR}/flash/diskio.c
    )

    set(DISKIO_INCLUDE
        ${CMAKE_CURRENT_LIST_DIR}/flash
    )
ELSE()
    set(DISKIO_SOURCE
        ${CMAKE_CURRENT_LIST_DIR}/mram/diskio.c
    )

    set(DISKIO_INCLUDE
        ${CMAKE_CURRENT_LIST_DIR}/mram
    )
ENDIF()

target_sources(FatFs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/source/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ffunicode.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ff.c
    
    # custom implemented diskio
    ${DISKIO_SOURCE}
)

target_include_directories(FatFs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/source

    # custom implemented diskio
    ${DISKIO_SOURCE}
)