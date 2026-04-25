// Implementation of the diskio functions for FatFs
// built using W25Q128JV
#include "ff.h" // needs to be included before diskio.h to get integer definitions
#include "diskio.h"
#include "flash.h"
#include "HardwareConfig.h"

static DSTATUS Stat = STA_NOINIT; 

DSTATUS disk_initialize (BYTE pdrv){
    (void)pdrv; // 1 drive system 

    flash_init(FS_SPI_BUS, FS_CS_PIN); 

    Stat = RES_OK;

    return RES_OK; // can't really fail
}

DSTATUS disk_status (BYTE pdrv){
    if(pdrv) return STA_NOINIT; // 1 drive system - drive 0

    return Stat; 
}

// read sectors from disk 
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count){
    (void)pdrv; // 1 drive system
    DWORD address = sector * SECTOR_SIZE; 

    for(int i = 0; i < count; i++){
        read_sector(address, buff); 

        buff += SECTOR_SIZE;
        address += SECTOR_SIZE; 
    }

    return 0;
}

// write sectors to disk 
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count){
    (void)pdrv; // 1 drive system
    DWORD address = sector * SECTOR_SIZE; 

    for(int i = 0; i < count; i++){
        write_sector(address, buff); 

        address += SECTOR_SIZE; 
        buff += SECTOR_SIZE;
    }
    
    return 0; 
}

// misc ioctl 
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff){
    (void)pdrv; // 1 drive system

    switch(cmd){
        case GET_SECTOR_COUNT: 
            *(DWORD*)buff = DEVICE_SIZE / SECTOR_SIZE; 
            return RES_OK; 
        case GET_SECTOR_SIZE:
            *(DWORD*)buff = SECTOR_SIZE; 
            return RES_OK; 
        case CTRL_SYNC: 
            return RES_OK; // this should be smarter 
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = SECTOR_SIZE; // how does FatFs differentiate between SECTOR and BLOCK 
            return RES_OK; 
        default: 
            return RES_PARERR; 
    }

    return RES_ERROR; 
}


