#ifndef DISKIO_TEST_H 
#define DISKIO_TEST_H

#include "ff.h"
#include "diskio.h"

int test_diskio (
    BYTE pdrv,      /* Physical drive number to be checked (all data on the drive will be lost) */
    UINT ncyc,      /* Number of test cycles */
    DWORD* buff,    /* Pointer to the working buffer */
    UINT sz_buff    /* Size of the working buffer in unit of byte */
);

#endif 