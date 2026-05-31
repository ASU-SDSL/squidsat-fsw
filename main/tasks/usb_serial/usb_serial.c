#include "usb_serial.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "tusb_config.h"
#include "tusb.h"

// Reference Arduino-Pico
// Reference https://github.com/hathach/tinyusb/blob/master/examples/device/cdc_msc_freertos/src/main.c

TaskHandle_t usbTaskHandle;
SemaphoreHandle_t usb_mutex;

void usb_serial_init(){
  usb_mutex = xSemaphoreCreateMutex();
  
}

void usb_serial_task(void *params){

  // should be called after scheduler is started
  tusb_init(); 

  while (1) {
    BaseType_t ss = xTaskGetSchedulerState(); 
    if(ss != taskSCHEDULER_SUSPENDED){

      if(usb_mutex != NULL && xSemaphoreTake(usb_mutex, 0) == pdTRUE){
        tud_task();
        xSemaphoreGive(usb_mutex);
      }

    }
    
    vTaskDelay(1); 
  }

}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
  
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if (dtr) {
    // Terminal connected
  } else {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) {
  (void) itf;
}