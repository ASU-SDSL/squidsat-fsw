#include "circular_log.h"

#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "timing.h"

#define cl_printf(...) printf(__VA_ARGS__); 

// used for making names clearer 
#define LOGS_BASE "/log"
#define MAX_ENTRY_LENGTH 128

int clog_create(){
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
        } else if(res == FR_NO_PATH || res == FR_NO_FILE) {
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
int clog_log(const char* fmt, ...){

    if(filesystem_start_use()){
        FIL fp; 
        FRESULT res = f_open(&fp, LOGS_BASE "/" LOGS_BASE ".txt", FA_OPEN_APPEND | FA_WRITE);

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
        int written = f_printf(&fp, "%s\n", buf); 

        if(written < 0){
            filesystem_end_use(); 
            return written; 
        }

        if(res != FR_OK){
            f_close(&fp);

            filesystem_end_use(); 
            return res; 
        }

        f_close(&fp);
        filesystem_end_use(); 
    } else {
        cl_printf("Failed to get filesystem use\n");
        return 1; 
    }

    return 0; 
}

void clog_dump() {
    if(filesystem_start_use()){
        FIL fp; 
        FRESULT res = f_open(&fp, LOGS_BASE "/" LOGS_BASE ".txt", FA_READ);

        if(res != FR_OK){
            filesystem_end_use(); 
            return; 
        }
        cl_printf("Logs:\n");
        while(f_eof(&fp) == false) {
            char buf[MAX_ENTRY_LENGTH+1];

            char* res_buf = f_gets(buf, sizeof(buf), &fp); 

            if(res_buf != buf) {
                cl_printf("Line Error\n"); 
            } else {
                cl_printf(buf); 
            }
        }
        cl_printf("Logs Done.\n"); 

        filesystem_end_use(); 
    } else {
        cl_printf("Failed to get filesystem use\n");
    }

    return; 
}

void clog_test() {
    printf("Starting circular log test...\n");

    // delete log file if it exists
    if(filesystem_start_use()){
        FILINFO fno;
        FRESULT res = f_stat(LOGS_BASE "/" LOGS_BASE ".txt", &fno);
        if(res == FR_OK) {
            res = f_unlink(LOGS_BASE "/" LOGS_BASE ".txt");
            if(res != FR_OK) {
                printf("Failed to delete log file, res: %d\n", res);
            }
        }
        filesystem_end_use();
    } else {
        printf("Failed to get filesystem use\n");
    }

    int res = clog_create();
    printf("Created log res %d\n", res); 

    printf("Starting logging\n"); 

    for(int i = 0; i < 10; i++){
        res = clog_log("This is log entry %d", i); 
        printf("\tLog res: %d\n", res);
    }

    clog_dump();

    printf("Done.\n"); 
}