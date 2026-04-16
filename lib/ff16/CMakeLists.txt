add_library(FatFs INTERFACE)

target_sources(FatFs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/source/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ffunicode.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ff.c
    # ${CMAKE_CURRENT_LIST_DIR}/source/diskio.c
)

target_include_directories(FatFs INTERFACE
    source
    include
)