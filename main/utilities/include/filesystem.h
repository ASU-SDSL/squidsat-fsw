// filesystem util and testing 
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "ff.h"

extern FATFS fs; 

void filesystem_init(); 
FRESULT filesystem_mkfs(); 
void filesystem_test(); 

#endif // FILESYSTEM_H