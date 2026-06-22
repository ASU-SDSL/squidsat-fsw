#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"

#include "timing.h"
#include "gse.h"
#include "log.h"

#include "ff.h"
#include "diskio.h"

#include "flash.h"
#include "HardwareConfig.h"

FATFS fs; // extern'ed


void diskio_test(){
    static uint8_t _it = 0; 

    printf("\n--------- Diskio test -----------\n");
    
    printf("Current AON Time: %lld\n", timing_now_epoch());
    int res; 

    if(_it == 0){
        res = flash_init(FS_SPI_BUS, FS_CS_PIN, FS_SPI_BAUDRATE); 
        printf("Flash init result: %d\n", res);
        vTaskDelay(10); // arbitrary delay
    }
    _it++; 

    int id = flash_read_id();
    printf("Flash ID: %x\n", id); 

    uint8_t status = flash_read_status1(); 
    printf("Status1: 0x%02x\n", status); 

    // -------------------------------------------------------------------
    // flash test
    // uint8_t buff[SECTOR_SIZE]; 

    // printf("Read\n"); 
    // res = read_sector(0, buff);
    // printf("res: %d\n", res);

    // for(int i = 0; i < 10; i++){
    //     printf("0x%02x ", buff[i]); 
    // }
    // printf("\n"); 

    // vTaskDelay(pdMS_TO_TICKS(1000)); 

    // printf("Write\n");
    // for(int i = 0; i < 5; i++){
    //     buff[i] = _it; 
    // }
    // res = write_sector(0, buff); 
    // printf("res: %d\n", res);

    // ----------------------------------------------------------------------

    uint8_t buff[SECTOR_SIZE]; 

    DRESULT read_res = disk_read(0, buff, 0, 1);
    printf("\nRead sector result: %d\n", read_res); 

    for(int i = 0; i < 10; i++){
        printf("0x%02x ", buff[i]);
    }
    printf("\n"); 

    vTaskDelay(pdMS_TO_TICKS(1000)); 

    // -----------------------------------------------------------------------

    uint8_t seed = _it; 
    for(int i = 0; i < 5; i++){
        buff[i] = seed; 
    }

    DRESULT write_res = disk_write(0, buff, 0, 1); 
    printf("\nWrite res (seed = 0x%02x): %d\n", seed, write_res); 

    vTaskDelay(pdMS_TO_TICKS(1000)); // wait for read to complete?

    // ---------------------------------------------------------------------

    read_res = disk_read(0, buff, 0, 1);
    printf("\nRead after write result: %d\n", read_res); 

    for(int i = 0; i < 10; i++){
        printf("0x%02x ", buff[i]);
    }
    printf("\n"); 

    // -----------------------------------------------------------------------

    vTaskDelay(pdMS_TO_TICKS(1000)); 

    printf("\n-------- Diskio test done. ----------\n"); 
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