#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "timing.h"
#include "gse.h"
#include "log.h"

#include "ff.h"
#include "diskio.h"

#define file_logf(...) printf(__VA_ARGS__)

/**
 * restrains the use of the file system to avoid using 
 * too many resources. Tasks that call FatFS functions 
 * should ask first to get an instance of the fs_use 
 * counting mutex AND handle rejection 
 * (but this is not enforced) 
 */
// overall filesystem lock - if locked do not get instance 
SemaphoreHandle_t fs_lock;
// instance management 
SemaphoreHandle_t fs_available; // instance management

#define FS_LOCK_DELAY_MS 1000 
#define FS_AVAILABLE_DELAY_MS 1000 

bool filesystem_start_use(uint32_t fs_lock_delay_ms, uint32_t fs_available_delay_ms){
    bool res = false; 
    if(xSemaphoreTake(fs_lock, fs_lock_delay_ms) == pdTRUE){
        res = xSemaphoreTake(fs_available, fs_available_delay_ms); 

        xSemaphoreGive(fs_lock); 
    }

    return res; 
}

void filesystem_end_use(){
    xSemaphoreGive(fs_available); 
}

// kind of arbitrary but some restriction is safer
#define FS_AVAILABLE_INSTANCES 4 

#define MKFS_WORKING_BUFFER_LEN FF_MAX_SS

// later adding resource management - locking mutex, counting mutex, drain 

FATFS fs; // extern'ed

void filesystem_init(){
    fs_available = xSemaphoreCreateCounting(FS_AVAILABLE_INSTANCES, FS_AVAILABLE_INSTANCES); 
    
    FRESULT fr = f_mount(&fs, "0:", 1); 
    if(fr != FR_OK){
        file_logf("Failed to mount filesystem (%d), attempting to rebuild\n", fr); 
        filesystem_mkfs(); 
    }
}

FRESULT filesystem_mkfs(){
    file_logf("Making filesystem\n");
    // make filesystem on drive 0 
    void* buf = pvPortMalloc(MKFS_WORKING_BUFFER_LEN); 
    FRESULT fr = f_mkfs("", NULL, buf, MKFS_WORKING_BUFFER_LEN); 
    // free buf 
    vPortFree(buf); 

    if(fr != FR_OK){
        file_logf("(mkfs) Failed to make filesystem (%d)\n", fr);    
        return fr; 
    }

    file_logf("Mounting filesystem\n"); 
    fr = f_mount(&fs, "0:", 1); 
    if(fr != FR_OK){
        file_logf("(mkfs) Failed to mount filesystem (%d)\n", fr); 
        return fr; 
    }

    return fr; // should only be FR_OK at this point 
}

void filesystem_test(){
    file_logf("Testing filesystem");
    vTaskDelay(pdMS_TO_TICKS(100)); 

    FRESULT fr = filesystem_mkfs(); 
    file_logf("MKFS res: %d\n", fr);

    file_logf("Done."); 
}

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