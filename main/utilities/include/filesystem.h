// filesystem util and testing 
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "ff.h"

extern FATFS fs; 

void diskio_test(); 
void filesystem_init(); 
void filesystem_make(); 
void filesystem_test(); 

#endif // FILESYSTEM_H