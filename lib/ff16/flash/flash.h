#ifndef FLASH_H
#define FLASH_H 

#include <stdint.h>
#include "hardware/spi.h"
#include "ff.h"

#define FS_SPI_BAUDRATE 1000000
#define DEVICE_SIZE 128000000 // actual 134217728 // bits
#define SECTOR_SIZE 4096

uint8_t flash_init(spi_inst_t* spi_bus, uint8_t cs); 

uint8_t read_sector(DWORD address, BYTE* buff); 

uint8_t write_sector(DWORD address, BYTE* buff); 

#endif // FLASH_H