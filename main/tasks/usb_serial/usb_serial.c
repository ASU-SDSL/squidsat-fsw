#include "usb_serial.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "tusb_config.h"
#include "tusb.h"

#include "pico/stdlib.h"
#include "pico/stdio/driver.h"

// Reference Arduino-Pico
// Reference https://github.com/hathach/tinyusb/blob/master/examples/device/cdc_msc_freertos/src/main.c

TaskHandle_t usbTaskHandle;
SemaphoreHandle_t usb_mutex;

static void usb_serial_out_chars(const char* buf, int len){
  if(usb_mutex != NULL && xSemaphoreTake(usb_mutex, 0) == pdTRUE){
    if(tud_cdc_connected()){
      tud_cdc_write(buf, len);
    }

    xSemaphoreGive(usb_mutex);
  }
}

static void usb_serial_out_flush(void){
  if(usb_mutex != NULL && xSemaphoreTake(usb_mutex, 0) == pdTRUE){
    if(tud_cdc_connected()){
      tud_cdc_write_flush();
    }

    xSemaphoreGive(usb_mutex);
  }
}

static int usb_serial_in_chars(char* buf, int len){
  if(usb_mutex != NULL && xSemaphoreTake(usb_mutex, 0) == pdTRUE){
    int count = 0; 
    if(tud_cdc_connected()){
      count = tud_cdc_read(buf, len);
    }

    xSemaphoreGive(usb_mutex);
    return count;
  }
  return 0;
}

// could I enable and then filter out usb to get pico tool? 
static void (*usb_serial_chars_available_fn)(void*); 
static void* usb_serial_chars_available_param;

// should be called in the usb task mutex
static void usb_serial_call_chars_available_callback(){
  if(tud_cdc_available()){
    if(usb_serial_chars_available_fn){
      usb_serial_chars_available_fn(usb_serial_chars_available_param);
    }
  }
}

static void usb_serial_set_chars_available_callback(void (*fn)(void*), void *param){
  usb_serial_chars_available_fn = fn; 
  usb_serial_chars_available_param = param; 
}

static stdio_driver_t usb_stdio_driver = {
  .out_chars = usb_serial_out_chars, // not used
  .out_flush = usb_serial_out_flush, // not used
  .in_chars = usb_serial_in_chars, // not used
  .set_chars_available_callback = usb_serial_set_chars_available_callback, // not used
  .next = NULL, // for the internal structure probably don't need to set it but whatever

#if PICO_STDIO_ENABLE_CRLF_SUPPORT // idk figure this out later 
  .last_ended_with_cr = false,
  .crlf_enabled = false
#endif
};

void usb_serial_init(){
  usb_mutex = xSemaphoreCreateMutex();
  
  // register custom usb driver 
  stdio_set_driver_enabled(&usb_stdio_driver, true);

}

void usb_serial_task(void *params){

  // should be called after scheduler is started
  tusb_init(); 

  while (1) {
    BaseType_t ss = xTaskGetSchedulerState(); 
    if(ss != taskSCHEDULER_SUSPENDED){

      if(usb_mutex != NULL && xSemaphoreTake(usb_mutex, 0) == pdTRUE){
        tud_task();

        usb_serial_call_chars_available_callback();
        xSemaphoreGive(usb_mutex);
      }

    }
    
    vTaskDelay(1); 
  }

}
