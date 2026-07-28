#ifndef MRAM_H
#define MRAM_H

#include "ff.h"
#include "hardware/spi.h"

#define DEVICE_SIZE 3200000 // bits // real is probably ~33554432 bits

// mram is made of 512-byte areas divisible down to 64-byte sections but I don't 
// think those are real sectors like flash
#define SECTOR_SIZE 512

int mram_init(spi_inst_t *spi_bus, uint cs, uint32_t spi_baud);

int read_sector(DWORD address, BYTE *buff);

int write_sector(DWORD address, const BYTE *buff);

int mram_read_id();

void mram_test(); 

#endif 