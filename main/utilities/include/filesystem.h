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

// -----------------------------------------------------------------------------
// filesystem helpers - for debugging NOT use in release code
// prints output with file_logf()
void filesystem_ls(const char* path);
void filesystem_stat(const char* path); 
void filesystem_dump(const char* path); 
void filesystem_dump_hex(const char* path);

#endif // FILESYSTEM_H