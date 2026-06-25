// filesystem util and testing 
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "ff.h"
#include <stdbool.h>

extern FATFS fs; 

void filesystem_init(); 
FRESULT filesystem_mkfs(); 
void filesystem_test(); 

bool filesystem_start_use(uint32_t fs_lock_delay_ms, uint32_t fs_available_delay_ms); 

bool filesystem_start_use_default(); 

void filesystem_end_use(); 

#endif // FILESYSTEM_H