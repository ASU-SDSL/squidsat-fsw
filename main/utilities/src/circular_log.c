#include "circular_log.h"

#include <stdio.h>

#include "filesystem.h"
#include "timing.h"

#define cl_printf(...) printf(__VA_ARGS__); 

// used for making names clearer 
#define LOGS_BASE "log"
#define MAX_ENTRY_LENGTH 128

int circular_log_create(){
    if(filesystem_start_use()) {

        // check if the logs directory exists and if it doesn't, create it 
        FILINFO fno; 

        FRESULT res = f_stat(LOGS_BASE, &fno);

        if(res == FR_OK) {
            if((fno.fattrib & AM_DIR) == 0) {
                // if it exists and isn't a directory 
                filesystem_end_use(); 
                return 1; 
            } 
        } else if(res == FR_NO_PATH) {
            // create it 
            res = f_mkdir(LOGS_BASE); 
            if(res != FR_OK){
                filesystem_end_use(); 
                return res; 
            }
        } else {
            filesystem_end_use(); 
            return res; // some other error 
        }

        filesystem_end_use(); 
    } else {
        printf("failed to get filesystem use\n");
        return 1; 
    }

    return 0; 
}

/**
 * @brief 
 * logs can be max 128 characters long, any more is discarded
 * 
 * @param fmt 
 * @param ... 
 * @return int 
 */
int circular_log(const char* fmt, ...){

    if(filesystem_start_use()){
        FIL fp; 
        FRESULT res = f_open(&fp, LOGS_BASE "/" LOGS_BASE ".txt", FA_OPEN_APPEND);

        if(res != FR_OK){
            filesystem_end_use(); 
            return 1; 
        }
        
        char buf[MAX_ENTRY_LENGTH];

        // create log 
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        // write log 
        res = f_printf(&fp, "%s\n", buf); 

        if(res != FR_OK){
            filesystem_end_use(); 
            return 1; 
        }

        filesystem_end_use(); 
    }

    return 0; 
}

void circular_log_dump() {
    if(filesystem_start_use()){
        FIL fp; 
        FRESULT res = f_open(&fp, LOGS_BASE "/" LOGS_BASE ".txt", FA_READ);

        if(res != FR_OK){
            filesystem_end_use(); 
            return; 
        }

        while(f_eof(&fp) == false) {
            char buf[MAX_ENTRY_LENGTH+1];

            char* res_buf = f_gets(buf, sizeof(buf), &fp); 

            if(res_buf != buf) {
                cl_printf("Line Error\n"); 
            } else {
                cl_printf(buf); 
            }
        }

        filesystem_end_use(); 
    }

    return; 
}