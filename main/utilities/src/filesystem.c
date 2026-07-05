/**
 * @file filesystem.c
 * @author Tyler Nielsen
 * @brief FatFs implementation for RTOS SMP with resource management. 
 * @version 0.1
 * @date 2026-06-24
 * 
 * @copyright Copyright (c) 2026
 * 
 * Restrains the use of the file system to avoid using too many resources. 
 * Tasks that call FatFS functions should ask first (with the public functions) 
 * to get an instance of the fs_use counting mutex AND handle rejection 
 * (but this is not enforced on fatfs functions)
 */
#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdbool.h>

#include "timing.h"
#include "gse.h"
#include "log.h"

#include "ff.h"
#include "diskio.h"

// tbd later
#define file_logf(...) printf(__VA_ARGS__)

/// Overall filesystem lock - if locked do not get instance 
static SemaphoreHandle_t fs_lock;
/// instance management & use record
static SemaphoreHandle_t fs_available; // instance management

/// Default claim delay for fs_lock 
#define FS_LOCK_DELAY_MS 1000 
/// Default claim delay for fs_available 
#define FS_AVAILABLE_DELAY_MS 1000 

/// Max number of fs_available claims
#define FS_AVAILABLE_INSTANCES 4 // kind of arbitrary but some restriction is safer
#define MKFS_WORKING_BUFFER_LEN FF_MAX_SS // must be >= FF_MAX_SS

/**
 * @brief Requests use instance of the filesystem. Called before any file system 
 * use. When the use is done call filesystem_end_use().
 * 
 * @param fs_lock_delay_ms Time to wait to take the fs_lock semaphore
 * @param fs_available_delay_ms Time to wait to take the fs_available semaphore
 * @return true if the fs_available semaphore is successfully taken 
 * @return false if either semaphore cannot be taken in the given times
 */
bool filesystem_start_use_args(uint32_t fs_lock_delay_ms, uint32_t fs_available_delay_ms){
    bool res = false; 
    if(fs_lock != NULL && xSemaphoreTake(fs_lock, fs_lock_delay_ms) == pdTRUE){
        if(fs_available != NULL) {
            res = xSemaphoreTake(fs_available, fs_available_delay_ms); 
        }

        xSemaphoreGive(fs_lock); 
    }
    
    return res; 
}

/**
 * @brief Default call to filesystem_start_use_args() with FS_LOCK_DELAY_MS 
 * and FS_AVAILABLE_DELAY_MS delays as arguments.
 * 
 * @return true 
 * @return false 
 */
bool filesystem_start_use(){
    return filesystem_start_use_args(FS_LOCK_DELAY_MS, FS_AVAILABLE_DELAY_MS);
}

/**
 * @brief Releases the fs_available mutex. Called at end of filesystem use.
 * 
 */
void filesystem_end_use(){
    if(fs_available != NULL) xSemaphoreGive(fs_available); 
}

/**
 * @brief For wrapping code blocks intended to be executed with nothing else 
 * using the fs. Should only be used inside filesystem.c, blocks at max delay
 * and waits for all uses of the fs_available to be yielded. 
 * 
 */
#define filesystem_with_max_lockout(code)                                      \
    if (fs_lock != NULL && xSemaphoreTake(fs_lock, portMAX_DELAY))         \
    {                                                                      \
        /* wait for current uses to be given up */                         \
        while (uxSemaphoreGetCount(fs_available) < FS_AVAILABLE_INSTANCES) \
        {                                                                  \
            vTaskDelay(pdMS_TO_TICKS(100));                                \
        }                                                                  \
        code                                                               \
            xSemaphoreGive(fs_lock);                                       \
    }

/// FatFs struct for overall use     
FATFS fs; // extern'ed

/**
 * @brief Initializes FatFs filesystem with resource management system. Attempts
 * to mount an existing filesystem, if that fails will attempt to make a new 
 * fileystem on the storage and mount that.
 * 
 * @return FRESULT FR_OK or pass along result of filesystem_build()
 */
FRESULT filesystem_init(){
    fs_lock = xSemaphoreCreateMutex(); 
    fs_available = xSemaphoreCreateCounting(FS_AVAILABLE_INSTANCES, FS_AVAILABLE_INSTANCES); 
    
    FRESULT fr = f_mount(&fs, "0:", 1); 
    if(fr != FR_OK){
        file_logf("Failed to mount filesystem (%d), attempting to rebuild\n", fr); 
        return filesystem_build(); 
    }

    return FR_OK;  
}

/**
 * @brief Builts and mounts a new filesystem on the storage device (1 drive 
 * system).
 * 
 * @return FRESULT Result of f_mkfs or f_mount if either fail, else FR_OK
 */
FRESULT filesystem_build(){
    FRESULT fr; 

    filesystem_with_max_lockout(

        file_logf("Making new filesystem\n");
        // make filesystem on drive 0 
        void* buf = pvPortMalloc(MKFS_WORKING_BUFFER_LEN); 
        fr = f_mkfs("", NULL, buf, MKFS_WORKING_BUFFER_LEN); 
        // free buf 
        vPortFree(buf); 

        if(fr != FR_OK){
            file_logf("(mkfs) Failed to make filesystem (%d)\n", fr);    
            return fr; 
        }

        file_logf("Mounting new filesystem\n"); 
        fr = f_mount(&fs, "0:", 1); 
        if(fr != FR_OK){
            file_logf("(mkfs) Failed to mount filesystem (%d)\n", fr); 
            return fr; 
        }

    )

    return fr; // should only be FR_OK at this point 
}

/**
 * @brief DEBUG ONLY - prints a list of items in a directory with file_logf().
 * Sourced from: https://elm-chan.org/fsw/ff/doc/readdir.html
 * @param path 
 */
void filesystem_ls(const char* path) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;


    res = f_opendir(&dir, path);                   /* Open the directory */
    if (res == FR_OK) {
        nfile = ndir = 0;
        for (;;) {
            res = f_readdir(&dir, &fno);           /* Read a directory item */
            if (fno.fname[0] == 0) break;          /* Error or end of dir */
            if (fno.fattrib & AM_DIR) {            /* It is a directory */
                file_logf("   <DIR>   %s\n", fno.fname);
                ndir++;
            } else {                               /* It is a file */
                file_logf("%10u %s\n", fno.fsize, fno.fname);
                nfile++;
            }
        }
        f_closedir(&dir);
        file_logf("%d dirs, %d files.\n", ndir, nfile);
    } else {
        file_logf("Failed to open \"%s\". (%u)\n", path, res);
    }
}

/**
 * @brief DEBUG ONLY - prints the stat of a file or directory with file_logf().
 * Sourced from: https://elm-chan.org/fsw/ff/doc/stat.html
 * @param path 
 */
void filesystem_stat(const char* path) {
    FRESULT fr;
    FILINFO fno;

    file_logf("Test for \"%s\"...\n", path);

    fr = f_stat(path, &fno);
    switch (fr) {

    case FR_OK:
        file_logf("Size: %lu\n", fno.fsize);
        file_logf("Timestamp: %u-%02u-%02u, %02u:%02u\n",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
        file_logf("Attributes: %c%c%c%c%c\n",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
        break;

    case FR_NO_FILE:
    case FR_NO_PATH:
        file_logf("\"%s\" is not exist.\n", path);
        break;

    default:
        file_logf("An error occurred. (%d)\n", fr);
    }
}

/**
 * @brief DEBUG ONLY - prints the contents of a file with file_logf() as text.
 * 
 * @param path 
 */
void filesystem_dump(const char* path){
    FRESULT fres; 
    FIL fil; 

    fres = f_open(&fil, path, FA_READ);
    if(fres != FR_OK){
        file_logf("Failed to open file \"%s\" for reading (%d)\n", path, fres);
        return; 
    }

    while(f_eof(&fil) == false){
        char buf[100]; 
        UINT count; 
        fres = f_read(&fil, buf, sizeof(buf) - 1, &count); 
        if(fres != FR_OK){
            file_logf("[Failed to read file \"%s\" (%d)]", path, fres);
            break; 
        }

        buf[count] = '\0'; // null terminate for safety
        file_logf("%s", buf); 
    }

    file_logf("\n"); 

    fres = f_close(&fil);
    if(fres != FR_OK){
        file_logf("Failed to close file \"%s\" after reading (%d)\n", path, fres);
    }
}

/**
 * @brief DEBUG ONLY - prints the contents of a file with file_logf() as hex.
 * 
 * @param path 
 */
void filesystem_dump_hex(const char* path){
FRESULT fres; 
    FIL fil; 

    fres = f_open(&fil, path, FA_READ);
    if(fres != FR_OK){
        file_logf("Failed to open file \"%s\" for reading (%d)\n", path, fres);
        return; 
    }

    while(f_eof(&fil) == false){
        char buf[100]; 
        UINT count; 
        fres = f_read(&fil, buf, sizeof(buf) - 1, &count); 
        if(fres != FR_OK){
            file_logf("[Failed to read file \"%s\" (%d)]", path, fres);
            break; 
        }

        for(int i = 0; i < count; i++){
            file_logf("%02X ", (unsigned char)buf[i]); 
        }
        
    }

    file_logf("\n"); 

    fres = f_close(&fil);
    if(fres != FR_OK){
        file_logf("Failed to close file \"%s\" after reading (%d)\n", path, fres);
    }
}

/**
 * @brief Test function for filesystem implementation
 * 
 */
void filesystem_test(){
    static bool needs_init = true; 

    file_logf("\n---------- Testing filesystem ----------\n");

    if(needs_init) {
        file_logf("Initializing filesystem...\n");
        filesystem_init(); 
        file_logf("Done\n");

        if(fs_lock != NULL) file_logf("\tfs_lock exists\n");
        if(fs_available != NULL) file_logf("\tfs_available exists\n"); 

        needs_init = false; 
    } else {
        file_logf("Filesytem already initialized\n");
    }

    vTaskDelay(pdMS_TO_TICKS(100)); 

    FRESULT fr = filesystem_build(); 
    file_logf("MKFS res: %d\n", fr);

    vTaskDelay(pdMS_TO_TICKS(100)); 

    if(filesystem_start_use()) {
        file_logf("Claimed filesystem use\n"); 

        // make file 
        FIL file; 
        fr = f_open(&file, "test.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE); 
        file_logf("open res: %d\n", fr); 

        // write to file 
        const char* buf = "Testing, testing. 1 2 3";
        UINT count;
        fr = f_write(&file, buf, strlen(buf) + 1, &count);
        file_logf("write res: %d count: %u\n", fr, count); 

        // move the file pointer back to top of the file 
        fr = f_lseek(&file, 0);
        file_logf("seek res: %d\n", fr); 

        // read from file 
        char* rbuf[30]; 
        fr = f_read(&file, rbuf, strlen(buf) + 1, &count);
        rbuf[29] = '\0'; // for safety
        file_logf("read res: %d count: %u\n", fr, count); 
        file_logf("Read: %s\n", rbuf);

        // close file 
        fr = f_close(&file); 
        file_logf("close res: %d\n", fr); 

        filesystem_ls("/");

        file_logf("\nStat test.txt: \n");

        filesystem_stat("test.txt");

        fr = f_open(&file, "test.txt", FA_READ); 
        file_logf("second open res: %d\n", fr);

        filesystem_dump("test.txt"); 

        f_close(&file);

        filesystem_end_use(); 

    } else {
        file_logf("Unable to claim filesystem use\n"); 
    }


    file_logf("Done.\n"); 
}

/**
 * @brief Gets the current time for file timestamps, declared in diskio.h. Not
 * intended for use outside of FatFS, see utilities/timing.c
 * 
 * @return DWORD 
 */
DWORD get_fattime(void){
    // struct tm now = {0}; 
    // if(rtc_get_tm(&now)){
    //     log_error("get_fattime RTC fail"); 
    // }
    struct tm now = timing_now_tm();

    return (DWORD)(now.tm_year - 80) << 25 |
           (DWORD)(now.tm_mon + 1) << 21 |
           (DWORD)now.tm_mday << 16 |
           (DWORD)now.tm_hour << 11 |
           (DWORD)now.tm_min << 5 |
           (DWORD)now.tm_sec >> 1;
}