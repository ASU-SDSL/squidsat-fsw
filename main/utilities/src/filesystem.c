#include "filesystem.h"

#include "gse.h"
#include "log.h"

#include "diskio.h"
#include "flash.h"
#include "HardwareConfig.h"

#include "ff.h"
#include "timing.h"

FATFS fs; // extern'ed

void diskio_test(){
    log_info("Diskio test");
    

    log_infof("Current AON Time: %lld", timing_now_epoch());

    uint8_t res = flash_init(FS_SPI_BUS, FS_CS_PIN, FS_SPI_BAUDRATE); 
    log_infof("Flash init result: %d", res);


    log_info("Diskio test done."); 
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

void filesystem_init(){
    
}

void filesystem_make(){
    log_info("Making filesystem");
    void* buf = pvPortMalloc(0x400); 
    FRESULT fr = f_mkfs("", NULL, buf, 0x400); 

}

void filesystem_test(){
    log_info("Testing filesystem");

    log_info("Done."); 
}