#include "circular_log.h"

#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "timing.h"

#define cl_printf(...) printf(__VA_ARGS__); 

// used for making names clearer 
// log naming needs to take into account buffer sizes use in the functions 
// (see LOG_FILE_PATH_BUFFER_SIZE) best to keep base names short (~4 chars)
#define LOGS_BASE "/log"
#define MAX_ENTRY_LENGTH 128
#define MAX_LOG_FILE_SIZE (MAX_ENTRY_LENGTH * 10)
#define MAX_LOG_FILE_INDEX 2
#define LOG_FILE_PATH_BUFFER_SIZE 20

// used to make macros into string constants 
#define FORCE_INTERPRET(x) #x
#define STRINGIFY(x) FORCE_INTERPRET(x)

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
        
        FILINFO fno;

        // check size of the file
        FRESULT fres = f_stat(LOGS_BASE "/" LOGS_BASE "0.txt", &fno);

        if(fres == FR_OK && fno.fsize > MAX_LOG_FILE_SIZE){
            // delete oldest log
            (void) f_unlink(LOGS_BASE "/" LOGS_BASE STRINGIFY(MAX_LOG_FILE_INDEX) ".txt");

            // cycle logs 
            for(int i = MAX_LOG_FILE_INDEX; i >= 0; i--){
                // ignore FR_NO_FILE - let fatfs just not do anything
                char old_name[LOG_FILE_PATH_BUFFER_SIZE]; 
                char new_name[LOG_FILE_PATH_BUFFER_SIZE]; 
                sprintf(old_name, LOGS_BASE "/" LOGS_BASE "%d.txt", i);
                sprintf(new_name, LOGS_BASE "/" LOGS_BASE "%d.txt", i+1);

                (void) f_rename(old_name, new_name);
            }
        }

        // open the base log for appending

        FIL fil; 
        fres = f_open(&fil, LOGS_BASE "/" LOGS_BASE "0.txt", FA_OPEN_APPEND | FA_WRITE);

        if(fres != FR_OK){
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
        int written = f_printf(&fil, "%s\n", buf); 

        if(written < 0){
            filesystem_end_use(); 
            return written; 
        }

        if(fres != FR_OK){
            f_close(&fil);

            filesystem_end_use(); 
            return fres; 
        }

        f_close(&fil);
        filesystem_end_use(); 
    } else {
        cl_printf("Failed to get filesystem use\n");
        return 1; 
    }

    return 0; 
}

int clog_dump(uint32_t index, uint32_t line_start, uint32_t line_stop) {
    // validate 
    if(index > MAX_LOG_FILE_INDEX){
        cl_printf("Invalid index: %d\n", index); 
        return 0; 
    }

    int line = 0; 

    if(filesystem_start_use()){
        FIL fil; 
        char log_file[LOG_FILE_PATH_BUFFER_SIZE];
        sprintf(log_file, LOGS_BASE "/" LOGS_BASE "%d.txt", index); 
        FRESULT res = f_open(&fil, log_file, FA_READ);

        if(res != FR_OK){
            filesystem_end_use(); 
            return -((int)res); 
        }
        cl_printf("Logs (%d: %d-%d):\n", index, line_start, line_stop);
        while(f_eof(&fil) == false && line <= line_stop) {
            char buf[MAX_ENTRY_LENGTH+1];

            char* res_buf = f_gets(buf, sizeof(buf), &fil); 

            if(line >= line_start) {
                if(res_buf != buf) {
                    cl_printf("Line Error\n"); 
                } else {
                    cl_printf(buf); 
                }
            }
            
            line++; 
        }
        cl_printf("Logs Done.\n"); 

        f_close(&fil); 

        filesystem_end_use(); 
    } else {
        cl_printf("Failed to get filesystem use\n");
    }

    return line - line_start; 
}

void clog_test() {
    printf("Starting circular log test...\n");

    printf("Make clean filesystem\n"); 
    filesystem_build(); 

    printf("Create log\n");
    int res = clog_create();
    printf("\tCreated log res %d\n", res); 

    printf("Starting logging\n"); 

    for(int j = 0; j < 4; j++){
        printf("\nIteration %d\n", j);
        printf("\tWrite logs...\n"); 
        for(int i = 0; i < 100; i++){
            res = clog_log("This is log entry %d", i); 
            if(res != 0){
                printf("Failed to log entry %d, res: %d\n", i, res); 
            }
        }
        printf("\tDone\n"); 

        // display directory 
        if(filesystem_start_use()){
            printf("Directory " LOGS_BASE ":\n");
            filesystem_ls(LOGS_BASE);
            printf("\n"); 

            filesystem_end_use(); 
        } else {
            printf("Failed to get filesystem use\n");
        }

        clog_dump(0, 6, 12);

    }

    printf("Done.\n"); 
}