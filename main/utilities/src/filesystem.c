#include "filesystem.h"

#include "gse.h"
#include "ff.h"
#include "rtc.h"

void diskio_test(){
    log_info("Diskio test");
    
    

    log_info("Diskio test done."); 
}

DWORD get_fattime(void){
    struct tm now = {0}; 
    if(rtc_get_tm(&now)){
        log_error("get_fattime RTC fail"); 
    }

    return (DWORD)(now.tm_year - 80) << 25 |
           (DWORD)(now.tm_mon + 1) << 21 |
           (DWORD)now.tm_mday << 16 |
           (DWORD)now.tm_hour << 11 |
           (DWORD)now.tm_min << 5 |
           (DWORD)now.tm_sec >> 1;
}

void filesystem_make(){
    log_info("Maxing filesystem");
    void* buf = pvPortMalloc(0x400); 
    FRESULT fr = f_mkfs("", NULL, buf, 0x400); 


}

void filesystem_init(){
    
}