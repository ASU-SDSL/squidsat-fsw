// Implementation of the diskio functions for FatFs
// built using W25Q128JV
#include "diskio.h"

#include "HardwareConfig.h"
#include "ff.h"  // needs to be included before diskio.h to get integer definitions
#include "mram.h"

static DSTATUS ff_stat = STA_NOINIT;

DSTATUS disk_status(BYTE pdrv) {
  if (pdrv) return STA_NOINIT;  // 1 drive system - drive 0

  return ff_stat;
}

DSTATUS disk_initialize(BYTE pdrv) {
  (void)pdrv;  // 1 drive system

  mram_init(FS_SPI_BUS, FS_CS_PIN, FS_SPI_BAUDRATE);

  ff_stat = RES_OK;  // can't really fail

  return ff_stat;
}

// read sectors from disk
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  (void)pdrv;  // 1 drive system

  DWORD address = sector * SECTOR_SIZE;

  for (int i = 0; i < count; i++) {
    int res = read_sector(address, buff);

    if (res != SECTOR_SIZE) return RES_PARERR;

    buff += SECTOR_SIZE;
    address += SECTOR_SIZE;
  }

  return RES_OK;
}

#if FF_FS_READONLY == 0 

// write sectors to disk
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  (void)pdrv;  // 1 drive system

  DWORD address = sector * SECTOR_SIZE;

  for (int i = 0; i < count; i++) {
    write_sector(address, buff);

    address += SECTOR_SIZE;
    buff += SECTOR_SIZE; 
  }

  return 0;
}

#endif 

// misc ioctl
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  (void)pdrv;  // 1 drive system

  switch (cmd) {
    case GET_SECTOR_COUNT:
      *(DWORD *)buff = DEVICE_SIZE / SECTOR_SIZE;
      return RES_OK;
    case GET_SECTOR_SIZE:
      *(DWORD *)buff = SECTOR_SIZE;
      return RES_OK;
    case CTRL_SYNC:
      return RES_OK;  // this could be smarter
    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1;  // in unit of sectors 
      return RES_OK;
    default:
      return RES_PARERR;
  }

  return RES_ERROR;
}
