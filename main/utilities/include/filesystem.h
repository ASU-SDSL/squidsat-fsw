// filesystem util and testing 
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "ff.h"
#include <stdbool.h>
#include <stdarg.h>

extern FATFS fs; 

FRESULT filesystem_init(); 
FRESULT filesystem_build(); 
void filesystem_test(); 

bool filesystem_start_use_args(uint32_t fs_lock_delay_ms, uint32_t fs_available_delay_ms); 
bool filesystem_start_use(); 
void filesystem_end_use(); 

#endif // FILESYSTEM_H