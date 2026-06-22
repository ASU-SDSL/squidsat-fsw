#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

#include "ff.h"
#include "hardware/spi.h"

#define DEVICE_SIZE 128000000  // actual 134217728 // bits
#define SECTOR_SIZE 4096
#define PAGE_SIZE 256

int flash_init(spi_inst_t *spi_bus, uint8_t cs, uint32_t spi_baud);

int read_sector(DWORD address, BYTE *buff);

int write_sector(DWORD address, const BYTE *buff);

int erase_sector(DWORD address);

int flash_read_id();
uint8_t flash_read_status1();

#endif  // FLASH_H